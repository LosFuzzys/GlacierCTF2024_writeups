from pathlib import Path

import os
import shutil
import subprocess
import time
import tempfile
from pwn import tube, Timeout


class TmuxDispatcher:
    def __init__(self, tmux_session_name: str):
        self.session_name = tmux_session_name
        self.tmux_bin = shutil.which("tmux")
        if self.tmux_bin is None:
            raise Exception("tmux not found")

        self.shell = os.environ.get("SHELL", "bash")

    def cmd(self, command: list[str], check: bool = True, silent: bool = False):
        cmd = [self.tmux_bin] + command
        return subprocess.run(
            cmd,
            check=check,
            stdout=subprocess.DEVNULL if silent else None,
            stderr=subprocess.DEVNULL if silent else None,
        )

    def session_cmd(self, cmd: str, args: list[str] = []):
        self.cmd([cmd] + ["-t", f"{self.session_name}"] + args)

    def has_session(self):
        return (
            self.cmd(
                ["has-session", "-t", self.session_name], check=False, silent=True
            ).returncode
            == 0
        )

    def create_session(self, twidth: int = None, theight: int = None):
        if not self.has_session():
            args = ["new-session", "-d", "-c", os.getcwd(), "-s", self.session_name]

            if twidth:
                args += ["-x", str(twidth)]
            if theight:
                args += ["-y", str(theight)]

            args += [self.shell]

            self.cmd(args)

    def kill_session(self):
        if self.has_session():
            self.session_cmd("send-keys", ["-H"] + ["03", "0a"])
            self.session_cmd("kill-session")

    def attach(self):
        term_program = os.environ.get("TERM_PROGRAM", None)
        if term_program is None:
            raise Exception("Please set $TERM_PROGRAM to your terminal emulator")

        res = subprocess.run(
            [term_program] + ["tmux", "attach-session", "-t", self.session_name],
            start_new_session=True,
            env=os.environ,
            stderr=subprocess.DEVNULL,
        )

        if res.returncode != 0:
            raise Exception(f"Failed to start {term_program}")

        """
        if os.fork() == 0:
            os.execve(
                term_cmd[0],
                term_cmd + ["tmux", "attach-session", "-t", self.session_name],
                os.environ,
            )
            exit(-1)
        """


class tmuxio:
    def __init__(self, command: list[str], x: int = None, y: int = None):
        self.command = command

        session_name = f"{self.__class__.__name__} - {Path(command[0]).stem}"
        self.tmux = TmuxDispatcher(session_name)

        tmpdir = Path(tempfile.mkdtemp())
        self.output_pipe_path = tmpdir / "cmd_output"

        self.twidth = x
        self.theight = y

        self.output_pipe = None
        self.pane = None
        self.input_tube = None
        self.output_tube = None

        self._start()

    def _start(self):
        # Create a pipe for the output of tee
        try:
            os.mkfifo(self.output_pipe_path)
        except OSError:
            print(f"Failed to create pipe at {self.output_pipe_path}")
            raise

        self.tmux.create_session(self.twidth, self.theight)

        self.input_tube = tube()
        self.input_tube.send_raw = self.send

        """
        stty raw
        stty opost
        stty -echo
        stty discard ^-
        stty kill ^-
        stty erase ^-
        stty start ^-
        stty stop ^-
        stty swtch ^-
        stty werase ^-"
        stty eof ^-
        stty eol ^-
        stty eol2 ^-
        stty lnext ^-
        stty quit ^-
        stty susp ^-
        stty rprnt ^-
        stty
        quit = <undef>; erase = <undef>; kill = <undef>; eof = <undef>; start = <undef>; stop = <undef>; susp = <undef>; rprnt = <undef>; werase = <undef>; lnext = <undef>; discard = <undef>;
        -brkint -icrnl -imaxbel
        -isig
        """
        self.sendline(
            b"stty 0:5:bf:8a3a:3:0:0:0:0:0:1:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0"
        )

        if self.twidth or self.theight:
            args = []
            if self.twidth:
                args += ["-x", str(self.twidth)]
            if self.theight:
                args += ["-y", str(self.theight)]

            self.tmux.session_cmd("resize-window", args)
            self.tmux.session_cmd("resize-pane", args)

        # self.sendline(b"stty eol z")
        # self.sendline(b"stty ignbrk")
        # self.sendline(b"stty igncr")

        # self.sendline(b"stty utf8")
        self.sendline(
            " ".join(self.command).encode()
        )

        # self.sendline(b"stty eol2 ^-")
        # self.sendline(b"stty intr ^-")
        # self.sendline(b"stty -brkint")

        # self.sendline(b"stty raw")
        # self.sendline(b"stty eof ^-")

        return self

    def close(self, keep_session: bool = False):
        if keep_session:
            return

        self.tmux.kill_session()

        if self.output_pipe is not None:
            self.output_pipe.close()

    def capture_pane(self, start: None | int = 0, end: None | int = 100):

        cmd  = ['tmux', "capture-pane", "-p", "-S", str(start), "-E", str(end), "-t" "0.0"]
        return subprocess.check_output(cmd)


    def tmux_attach(self):
        self.tmux.attach()

    def read(self, n: int | None = None):
        return self.output_pipe.read(n)

    def rcev(self, n: int | None = None):
        return self.output_pipe.recv(n)

    def send(self, data: bytes):
        self.tmux.session_cmd("send-keys", ["-H"] + [f"{b:02x}" for b in data])

    def readuntil(self, delim: bytes, timeout: None | float = Timeout.default):
        return self.output_tube.readuntil(delim, timeout=timeout)

    def recvline(self, timeout: None | float = Timeout.default):
        return self.output_tube.readline(timeout=timeout)

    def readline(self, timeout: None | float = Timeout.default):
        return self.output_tube.readline(timeout=timeout)

    def readall(self, timeout: None | float = Timeout.default):
        return self.output_tube.readall(timeout=timeout)

    def sendline(self, line: bytes):
        self.input_tube.sendline(line)

    def dramatic_send(self, line: bytes, delay: float = 0.1):
        for c in line:
            time.sleep(delay)
            self.input_tube.send(c.to_bytes(1))

    def sendafter(
        self, delim: bytes, line: bytes, timeout: None | float = Timeout.default
    ):
        print(f"Reading until: {delim}")
        read = self.output_tube.readuntil(delim, timeout=timeout)
        self.input_tube.send(line)
        return read

    def sendlineafter(
        self, delim: bytes, line: bytes, timeout: None | float = Timeout.default
    ):
        self.sendafter(delim, line + b"\n", timeout=timeout)
