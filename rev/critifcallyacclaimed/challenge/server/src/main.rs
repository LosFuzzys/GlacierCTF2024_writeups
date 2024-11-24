use std::{error::Error, net::SocketAddr, time::Duration};

#[cfg(not(feature = "ipv4"))]
use std::net::{Ipv6Addr, SocketAddrV6};

#[cfg(feature = "ipv4")]
use std::net::{Ipv4Addr, SocketAddrV4};

use base64::{prelude::BASE64_STANDARD, Engine};
use bson::Document;
use critically_acclaimed_lib::{
    game::{Game, GameState, MechanicTimer},
    Direction, Mechanic,
};
use flate2::{write::ZlibEncoder, Compression};
use tokio::{
    fs::read,
    io::{AsyncBufReadExt, AsyncWriteExt, BufReader, BufWriter},
    net::{
        tcp::{ReadHalf, WriteHalf},
        TcpListener, TcpStream,
    },
};

#[cfg(not(feature = "silent"))]
use log::{debug, error, info, warn};

#[cfg(feature = "silent")]
#[macro_use]
mod silent;

#[cfg(not(feature = "ipv4"))]
const ADDRESS: Ipv6Addr = Ipv6Addr::new(0, 0, 0, 0, 0, 0, 0, 0);
#[cfg(feature = "ipv4")]
const ADDRESS: Ipv4Addr = Ipv4Addr::new(0, 0, 0, 0);
const PORT: u16 = 1337;
const MECHANICS_TIME_DELTA: usize = 3;
const THREAD_ALARM: Duration = Duration::from_secs(60);
const PLAYER_ACTIONS: usize = 10;

#[tokio::main]
async fn main() {
    #[cfg(not(feature = "silent"))]
    env_logger::init();

    #[cfg(not(feature = "ipv4"))]
    let listener = TcpListener::bind(SocketAddrV6::new(ADDRESS, PORT, 0, 0))
        .await
        .unwrap();

    #[cfg(feature = "ipv4")]
    let listener = TcpListener::bind(SocketAddrV4::new(ADDRESS, PORT))
        .await
        .unwrap();

    info!("Listening on [{}]:{}", ADDRESS, PORT);
    loop {
        match listener.accept().await {
            Ok((client, addr)) => {
                handle_connection(client, addr);
            }
            Err(_err) => {
                error!("Error while connecting client: {:?}", _err);
            }
        }
    }
}

fn handle_connection(client: TcpStream, _addr: SocketAddr) {
    info!("New connection from {}", _addr);
    tokio::spawn(async move {
        let result = tokio::time::timeout(THREAD_ALARM, handle_client(client, _addr)).await;
        match result {
            Ok(join_handle) => {
                let _ = join_handle.map_err(|_e| warn!("Client {} exited with '{}'", _addr, _e));
            }
            Err(_) => warn!(
                "Client {} timed out after {}s",
                _addr,
                THREAD_ALARM.as_secs_f32()
            ),
        }
    });
}

async fn handle_client(mut client: TcpStream, _addr: SocketAddr) -> Result<(), Box<dyn Error>> {
    debug!("{}: Handling new client", _addr);
    client.set_nodelay(true)?;
    let (reader, writer) = client.split();
    let reader = BufReader::new(reader);
    let writer = BufWriter::new(writer);
    let result = start_game(reader, writer, _addr).await?;
    debug!("{}: Got result", _addr);
    client.write_all("\n\n".as_bytes()).await?;
    if result {
        debug!("{}: We have a winner!", _addr);
        client.write_all("\n".as_bytes()).await?;
        client.write_all(read("flag.txt").await?.as_slice()).await?;
        client.write_all("\n".as_bytes()).await?;
    } else {
        client.write_all("Skill issue!\n".as_bytes()).await?;
    }
    client.shutdown().await?;
    Ok(())
}

async fn write_mechancis(
    to_send: Vec<&MechanicTimer>,
    writer: &mut BufWriter<WriteHalf<'_>>,
) -> Result<(), Box<dyn Error>> {
    let serialized = bson::to_bson(&to_send)?;
    let mut doc = Document::new();
    doc.insert("game", serialized);
    let mut bson_data = Vec::new();
    doc.to_writer(&mut bson_data)?;
    let mut zlibenc = ZlibEncoder::new(Vec::new(), Compression::default());
    std::io::Write::write_all(&mut zlibenc, &bson_data)?;
    writer
        .write_all(
            BASE64_STANDARD
                .encode(zlibenc.finish()?.as_slice())
                .as_bytes(),
        )
        .await?;
    writer.write_all(b"\x0a").await?;
    writer.flush().await?;
    Ok(())
}

async fn start_game(
    mut reader: BufReader<ReadHalf<'_>>,
    mut writer: BufWriter<WriteHalf<'_>>,
    _addr: SocketAddr,
) -> Result<bool, Box<dyn Error>> {
    debug!("{}: Game started", _addr);
    let mut buf = Vec::with_capacity(256);
    let mut loop_counter: usize = 0;
    // New game loop
    loop {
        debug!("{}: New game", _addr);
        let mut game = Game::new(((0, 0).into(), (32, 32).into()), 32);
        game.player.force_move_to((15, 15).into());
        let mechanics_raw: Vec<Vec<Mechanic>> =
            serde_yml::from_str(String::from_utf8(read("mechanics.yml").await?)?.as_str())?;
        let mut mechanics = Vec::new();
        for (t, mechanic_group) in mechanics_raw.iter().enumerate() {
            mechanics.extend(mechanic_group.iter().map(|m| MechanicTimer::new(*m, t)));
        }
        debug!("{}: Mechanics read", _addr);

        game.mechanic_queue.extend(mechanics);
        let to_send: Vec<&MechanicTimer> = game
            .mechanic_queue
            .iter()
            .filter(|mt| mt.timer <= MECHANICS_TIME_DELTA)
            .collect();
        write_mechancis(to_send, &mut writer).await?;
        debug!("{}: Sent initial mechanics", _addr);

        // Running game turn loop
        loop {
            debug!("{}: New tick", _addr);
            buf.clear();
            let mut damage_boss = false;
            reader.read_until(0x0a, &mut buf).await?;
            let player_input = String::from_utf8_lossy(&buf.as_slice());

            // Player input loop
            for c in player_input.chars().take(PLAYER_ACTIONS) {
                debug!("{}", c);
                match c {
                    'u' => game.player.player_move(Direction::N),
                    'd' => game.player.player_move(Direction::S),
                    'l' => game.player.player_move(Direction::W),
                    'r' => game.player.player_move(Direction::E),
                    'a' => {
                        damage_boss = true;
                        break;
                    }
                    'h' => {
                        game.player.heal();
                        break;
                    }
                    'b' => {
                        game.player.block();
                        break;
                    }
                    'q' => return Ok(false),
                    '💎' => {
                        writer.write_all("SPHEEEEEEEENEE".as_bytes()).await?;
                        writer
                            .write_all("\nSPHENE LISTEN TO ME\n".as_bytes())
                            .await?;
                    }
                    _ => (),
                }
            } // Player input loop

            debug!("{}: Advancing game tick", _addr);
            game.advance()?;

            // Damage after advance or we can kill the boss at the start of a tick
            // This will cause an error and possibly panic
            // Not enough time to fix properly
            if damage_boss {
                game.damage_boss();
            }

            match game.check_state() {
                GameState::Victory => return Ok(true),
                GameState::Defeat => break,
                GameState::Running => (),
            }

            let to_send: Vec<&MechanicTimer> = game
                .mechanic_queue
                .iter()
                .filter(|mt| mt.timer == game.tick + MECHANICS_TIME_DELTA)
                .collect();
            write_mechancis(to_send, &mut writer).await?;
        } // Running game turn loop

        writer
            .write_all(format!("l{:02}\n", loop_counter % 100).as_bytes())
            .await?;
        loop_counter += 1;
    } // New game loop
}
