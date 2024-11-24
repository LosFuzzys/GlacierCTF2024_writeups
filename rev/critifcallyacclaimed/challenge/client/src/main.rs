#[allow(unused_imports)]
use std::f32::consts::PI;
#[allow(unused_imports)]
use std::{fs::write, mem};
#[allow(unused_imports)]
use std::{
    fs::{read, File},
    io::{stdin, Read, Write},
};

#[allow(unused_imports)]
use bson::{from_bson, Document};
use critically_acclaimed_lib::game::GameState;

#[allow(unused_imports)]
use critically_acclaimed_lib::{
    game::{Game, MechanicTimer},
    Direction, Mechanic,
};

const EMPTY_TILE: &str = ". ";
const AOE_TILE: &str = "██";
const ARENA_SIZE: usize = 32;

fn apply_mechanic(grid: &mut [[&str; ARENA_SIZE]; ARENA_SIZE], mechanic: Mechanic) {
    match mechanic {
        Mechanic::Tower { pos } => grid[pos.x as usize][pos.y as usize] = "TT",
        Mechanic::Flare { pos } => grid[pos.x as usize][pos.y as usize] = "FL",
        Mechanic::Wind { orientation } => print!("wind ({}), ", orientation),
        Mechanic::Knockback { pos } => grid[pos.x as usize][pos.y as usize] = "KB",
        Mechanic::Gaze { pos } => grid[pos.x as usize][pos.y as usize] = "GZ",
        Mechanic::Raidwide { damage: _ } => print!("raidwide, "),
        Mechanic::Pyretic => print!("pyretic, "),
        Mechanic::Enrage => print!("enrage, "),
        _ => apply_limited_area_mechanic(grid, mechanic),
    }
}

fn apply_limited_area_mechanic(grid: &mut [[&str; ARENA_SIZE]; ARENA_SIZE], mechanic: Mechanic) {
    for (x, a) in grid.iter_mut().enumerate() {
        for (y, c) in a.iter_mut().enumerate() {
            if *c == EMPTY_TILE && mechanic.resolve((x, y).try_into().unwrap()).unwrap() {
                *c = AOE_TILE;
            }
        }
    }
}

fn display(grid: [[&str; ARENA_SIZE]; ARENA_SIZE]) {
    for y in (0..grid.len()).rev() {
        print!("{} ", y % 10);
        for x in 0..grid[0].len() {
            print!("{}", grid[x][y]);
        }
        println!();
    }
    print!("x ");
    for i in 0..grid.len() {
        print!("{} ", i % 10);
    }
    println!();
}

#[allow(dead_code)]
fn display_one(mechanics: Vec<Mechanic>, player_pos: (i32, i32)) {
    let mut grid = [[EMPTY_TILE; ARENA_SIZE]; ARENA_SIZE];
    let mut game = Game::new(
        ((0, 0).into(), (ARENA_SIZE as i32, ARENA_SIZE as i32).into()),
        10,
    );
    game.player.force_move_to(player_pos.into());

    grid[game.player.pos().x as usize][game.player.pos().y as usize] = "--";

    game.mechanic_queue
        .extend(mechanics.iter().map(|m| MechanicTimer::new(*m, 0)));
    game.advance().unwrap();

    print!("Additional mechanics in effect: ");
    for mechanic in mechanics {
        apply_mechanic(&mut grid, mechanic);
    }
    println!();

    if game.is_player_in_bounds() {
        grid[game.player.pos().x as usize][game.player.pos().y as usize] = "&&";
    } else {
        println!(
            "Player out of bounds! x: {}, y: {}",
            game.player.pos().x,
            game.player.pos().y
        );
    }
    display(grid);

    println!(
        "Player has {} HP and is {}",
        game.player.health(),
        if game.player.is_alive() {
            "alive"
        } else {
            "dead"
        }
    );
    println!("Game is {:#?}", game.check_state());
}

#[allow(dead_code)]
fn display_game(mechanics: Vec<MechanicTimer>, mut movement: Vec<((i32, i32), char, i32)>) {
    let mut game = Game::new(
        ((0, 0).into(), (ARENA_SIZE as i32, ARENA_SIZE as i32).into()),
        32,
    );

    game.player.force_move_to((15, 15).into());
    game.mechanic_queue.extend(mechanics);
    movement.reverse();

    while !game.mechanic_queue.is_empty() {
        let mut damage_boss = false;
        let mut moved_for = 0;
        if let Some((m, action, dir)) = movement.pop() {
            moved_for = (game.player.pos() - m.into()).abs().sum();
            if game.player.pos() != m.into() {
                game.player.force_set_moved();
                match (dir, (game.player.pos() - m.into()).signum().into()) {
                    (1, _) => game.player.force_set_direction(Direction::N),
                    (2, _) => game.player.force_set_direction(Direction::S),
                    (3, _) => game.player.force_set_direction(Direction::W),
                    (4, _) => game.player.force_set_direction(Direction::E),
                    (0, (_, -1)) => game.player.force_set_direction(Direction::N),
                    (0, (_, 1)) => game.player.force_set_direction(Direction::S),
                    (0, (-1, _)) => game.player.force_set_direction(Direction::W),
                    (0, (1, _)) => game.player.force_set_direction(Direction::E),
                    _ => (),
                }
            }
            game.player.force_move_to(m.into());
            match action {
                'a' => damage_boss = true,
                'h' => game.player.heal(),
                'b' => game.player.block(),
                'n' => (),
                _ => panic!("Invalid player action"),
            }
        }
        let mut grid = [[EMPTY_TILE; ARENA_SIZE]; ARENA_SIZE];
        grid[game.player.pos().x as usize][game.player.pos().y as usize] = "--";

        print!("Additional mechanics in effect: ");
        for mechanic in game
            .mechanic_queue
            .iter()
            .filter(|mt| mt.timer <= game.tick)
            .map(|mt| mt.mechanic)
        {
            apply_mechanic(&mut grid, mechanic);
        }
        println!();

        game.advance().unwrap();

        // Damage after advance or we can kill the boss at the start of a tick
        // This will cause an error and possibly panic
        // Not enough time to fix properly
        if damage_boss {
            game.damage_boss();
        }

        if game.is_player_in_bounds() {
            grid[game.player.pos().x as usize][game.player.pos().y as usize] = "&&";
        } else {
            println!(
                "Player out of bounds! x: {}, y: {}",
                game.player.pos().x,
                game.player.pos().y
            );
        }
        display(grid);
        println!("Tick {}", game.tick - 1);
        println!(
            "Player has {} HP and is {}. The player moved for {} tiles",
            game.player.health(),
            if game.player.is_alive() {
                "alive"
            } else {
                "dead"
            },
            moved_for
        );
        println!(
            "Game is {:#?} (boss has {} HP)",
            game.check_state(),
            game.boss_health
        );
        match game.check_state() {
            GameState::Running => (),
            _ => return,
        }
        let _ = stdin().read(&mut [0u8; 2]).unwrap();
    }
}

fn main() {
    /* Single */
    // let mechanics = [
    //     // Mechanic::Aoe { pos: (10, 10).into(), size: (5) },
    //     // Mechanic::Donut { pos: (10, 10).into(), size: 10, inner_size: 3 },
    //     // Mechanic::Donut { pos: (13, 15).into(), size: 6, inner_size: 3 },
    //     // Mechanic::Laser { orientation: PI*0.75, offsetx: 6., offsety: 3., thickness: 3. },
    //     // Mechanic::Laser { orientation: PI*0.6, offsetx: 20., offsety: 22., thickness: 2. },
    //     // Mechanic::Laser { orientation: 0.2617994f32, offsetx: 2., offsety: 5., thickness: 7. },
    //     // Mechanic::Laser { orientation: 1.8325958f32, offsetx: 0., offsety: -3., thickness: 7. },
    //     // Mechanic::Laser { orientation: 3.4033923f32, offsetx: 26., offsety: 28., thickness: 7. },
    //     // Mechanic::Laser { orientation: 4.974189f32, offsetx: -39., offsety: 15., thickness: 7. },
    //     // Mechanic::Cleave { orientation: PI * 0.5, start: -5, end: 15 },
    //     // Mechanic::Cleave { orientation: 0., start: 3, end: 10 },
    //     // Mechanic::Tower { pos: (15, 15).into() },
    //     // Mechanic::Gaze { pos: (20, 6).into() },
    //     // Mechanic::Wind { orientation: PI/4. },
    //     // Mechanic::Pyretic,
    // ];
    // display_one(mechanics.to_vec(), (1, 1));

    /* Multiple */
    // let mechanics = [
    //     MechanicTimer::new(Mechanic::Donut { pos: (8, 16).into(), size: 12, inner_size: 3 }, 0),
    //     MechanicTimer::new(Mechanic::Aoe { pos: (8,16).into(), size: (3)}, 1) ,
    //     MechanicTimer::new(Mechanic::Laser { orientation: PI*0.5, offsetx: 0., offsety: 6., thickness: 6.}, 2),
    //     MechanicTimer::new(Mechanic::Laser { orientation: PI*0.5, offsetx: 0., offsety: 26., thickness: 6.}, 2),
    //     MechanicTimer::new(Mechanic::Cleave { orientation: PI*0., start: -6, end: 20}, 3),
    //     MechanicTimer::new(Mechanic::Donut { pos: (15,15).into(), size:12, inner_size: 5}, 4),
    //     MechanicTimer::new(Mechanic::Tower { pos: (30,30).into() }, 5),
    //     MechanicTimer::new(Mechanic::Aoe { pos: (25, 5).into(), size: (10) }, 6),
    //     MechanicTimer::new(Mechanic::Aoe { pos: (5, 25).into(), size: (10) }, 6),
    //     MechanicTimer::new(Mechanic::Wind { orientation: 0. }, 7),
    //     MechanicTimer::new(Mechanic::Aoe { pos: (25, 25).into(), size: (10) }, 8),
    //     MechanicTimer::new(Mechanic::Aoe { pos: (5, 5).into(), size: (10) }, 8),
    // ];
    // display_game(mechanics.to_vec(), [].to_vec());

    /* Serde test */
    // let serialized = serde_json::to_string(&mechanics).unwrap();
    // let serialized = serde_json::to_string(&movement).unwrap();

    /* BSON */
    // let test: Vec<&MechanicTimer> = mechanics.iter().collect();
    // let serialized = bson::to_bson(&test).unwrap();
    // println!("serialized = {}", serialized);
    // let mut doc = Document::new();
    // doc.insert("game", serialized);
    // let mut bson_data = Vec::new();
    // doc.to_writer(&mut bson_data).unwrap();
    // write("test.bson", bson_data.as_slice()).unwrap();

    // let file = File::open("test.bson").unwrap();
    // let mut doc = Document::from_reader(file).unwrap();
    // println!("{}", doc);
    // let mechanics: Vec<MechanicTimer> = from_bson(mem::take(doc.get_mut("test").unwrap())).unwrap();
    // drop(doc);
    // println!("{:#?}", mechanics);

    /* SERIALIZATION */
    let mechanics_raw: Vec<Vec<Mechanic>> = serde_yml::from_str(
        String::from_utf8(read("mechanics.yml").expect("File mechanics.yml could not be read"))
            .expect("Mechanics UTF-8 decoding error")
            .as_str(),
    )
    .expect("Mechanics YML decoding error");
    let mut mechanics = Vec::new();
    for (t, mechanic_group) in mechanics_raw.iter().enumerate() {
        mechanics.extend(mechanic_group.iter().map(|m| MechanicTimer::new(*m, t)));
    }

    let movement: Vec<((i32, i32), char, i32)> = serde_json::from_str(
        String::from_utf8(
            read("player_movement.json").expect("File player_movement.json could not be read"),
        )
        .expect("Movement UTF-8 decoding error")
        .as_str(),
    )
    .expect("Movement JSON decoding error");

    display_game(mechanics, movement);

    // for i in 0..32 {
    //     println!("{}: {}", i, ((3. / (i as f32 + 2.5).log10().powf(5.)) * 70f32) as i32);
    // }
}
