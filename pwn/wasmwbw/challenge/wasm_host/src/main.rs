use anyhow::anyhow;
use base64::{engine::general_purpose as base64_engine, Engine as _};
use std::fs::File;
use std::io::{Read, Seek, SeekFrom, Write};
use std::sync::{Arc, Mutex};
use wasmtime::*;

fn wbw(r#where: i64, byte: i32) {
    let mut mem_file = File::options()
        .read(false)
        .write(true)
        .open("/proc/self/mem")
        .expect("Could not open /proc/self/mem");

    let self_offset = wbw as *const () as u64;
    let ret_instructions: [u8; 4] = [0xc3; 4];
    mem_file.seek(SeekFrom::Start(self_offset)).unwrap();
    let written = mem_file
        .write(&ret_instructions)
        .expect("Failed to clear wbw");

    if written != ret_instructions.len() {
        panic!("Failed to clear wbw");
    }

    println!("Writing 0x{:x} to 0x{:x}", byte, r#where);

    mem_file.seek(SeekFrom::Start(r#where as u64)).unwrap();

    let written = match mem_file.write(&[(byte & 0xff) as u8]) {
        Ok(written) => written,
        Err(e) => {
            println!("write error: {:?}", e);
            return;
        }
    };

    if written != 1 {
        println!("Failed to write to 0x{:x}", r#where);
    }
}

fn main() -> anyhow::Result<()> {
    println!("Enter your module in base64:");
    let user_module = {
        let stdin = std::io::stdin();
        let mut module_b64 = String::new();
        stdin.read_line(&mut module_b64)?;

        let module_b64 = module_b64.strip_suffix("\n").unwrap_or(&module_b64);

        match base64_engine::STANDARD.decode(module_b64) {
            Ok(m) => m,
            Err(_) => return Err(anyhow!("invalid base64")),
        }
    };

    let mut config = Config::new();

    config.debug_info(false);
    config.cranelift_debug_verifier(false);
    config.strategy(Strategy::Cranelift);
    config.cranelift_opt_level(OptLevel::SpeedAndSize);
    // Last year we had 35 bits. Naturally we needed to step it up this year.
    config.wasm_memory64(true);
    config.coredump_on_trap(true);

    let engine = Engine::new(&config)?;

    let user_module = match Module::new(&engine, user_module) {
        Ok(user_module) => user_module,
        Err(_) => return Err(anyhow!("invalid module")),
    };

    let mut store = Store::new(&engine, ());

    let proc_mem_file = File::options()
        .read(true)
        .write(false)
        .open("/proc/self/mem")
        .expect("Could not open /proc/self/mem");

    let proc_mem_file = Arc::new(Mutex::new(proc_mem_file));

    let inspect = Func::wrap(&mut store, move |r#where: i64| -> i64 {
        let mut mem_file = proc_mem_file.lock().unwrap();
        mem_file.seek(SeekFrom::Start(r#where as u64)).unwrap();

        #[cfg(debug_assertions)]
        println!("Reading from from 0x{:x}", r#where);

        let mut buffer = [0u8; 8];
        match mem_file.read_exact(&mut buffer) {
            Ok(_) => (),
            Err(e) => {
                if e.to_string().contains("Input/output error") {
                    return -1;
                }
                println!("read error: {:?}", e);
                return -2;
            }
        }

        i64::from_le_bytes(buffer)
    });

    let write_byte_where = Func::wrap(&mut store, wbw);

    let instance = Instance::new(
        &mut store,
        &user_module,
        &[inspect.into(), write_byte_where.into()],
    )?;

    let ret = instance
        .get_typed_func::<(), i32>(&mut store, "entry")?
        .call(&mut store, ())?;

    println!("Module returned: {}", ret);

    Ok(())
}
