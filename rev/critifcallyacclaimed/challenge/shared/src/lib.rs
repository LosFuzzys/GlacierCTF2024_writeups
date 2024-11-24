pub mod game;
mod mechanic;
pub mod player;
mod point;

#[cfg(feature = "serialize")]
use serde::Serialize;
#[cfg(feature = "deserialize")]
use serde::Deserialize;

pub use crate::mechanic::*;
pub use crate::point::Point;

pub(crate) const ARENA_SIZE: u32 = 32;
pub(crate) const EPSILON: f32 = 0.7;
pub(crate) const ROUND: f32 = 10000.;

#[cfg_attr(feature = "debug", derive(Debug))]
#[cfg_attr(feature = "serialize", derive(Serialize))]
#[cfg_attr(feature = "deserialize", derive(Deserialize))]
#[derive(Copy, Clone)]
pub enum Direction {
    N,
    S,
    E,
    W,
}

#[cfg(test)]
mod tests {
    use std::f32::consts::PI;

    use game::{Game, MechanicTimer};

    use super::*;

    const EMPTY_TILE: &str = ". ";
    const AOE_TILE: &str = "██";
    const ARENA_SIZE: usize = 32;
    
    fn apply_limited_area_mechanic(grid: &mut [[&str; ARENA_SIZE]; ARENA_SIZE], mechanic: Mechanic) {
        for (x, a) in grid.iter_mut().enumerate() {
            for (y, c) in a.iter_mut().enumerate() {
                if *c == EMPTY_TILE && mechanic.resolve((x, y).try_into().unwrap()).unwrap() {
                    *c = AOE_TILE;
                }
            }
        }
    }

    fn display(mechanic: Mechanic, player_pos: Point) {
        let mut grid = [[EMPTY_TILE; ARENA_SIZE]; ARENA_SIZE];

        match mechanic {
            Mechanic::Tower { pos } => grid[pos.x as usize][pos.y as usize] = "TT",
            Mechanic::Flare { pos } => grid[pos.x as usize][pos.y as usize] = "FL",
            Mechanic::Wind { orientation } => print!("wind ({}), ", orientation),
            Mechanic::Knockback { pos } => grid[pos.x as usize][pos.y as usize] = "KB",
            Mechanic::Gaze { pos } => grid[pos.x as usize][pos.y as usize] = "GZ",
            Mechanic::Raidwide { damage: _ } => print!("raidwide, "),
            Mechanic::Pyretic => print!("pyretic, "),
            Mechanic::Enrage => print!("enrage, "),
            _ => apply_limited_area_mechanic(&mut grid, mechanic),
        }
        grid[player_pos.x as usize][player_pos.y as usize] = "&&";

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

    #[test]
    fn aoe_hit_direct() {
        let m = Mechanic::Aoe { pos: (1, 1).into(), size: 3 };
        let p: Point = (1, 1).into();
        display(m, p);
        assert!(m.resolve(p).unwrap())
    }

    #[test]
    fn aoe_hit_near() {
        let m = Mechanic::Aoe { pos: (8, 8).into(), size: 4 };
        let p = (7, 6).into();
        display(m, p);
        assert!(m.resolve(p).unwrap())
    }

    #[test]
    fn aoe_miss() {
        let m =  Mechanic::Aoe { pos: (10, 10).into(), size: 5 };
        let p: Point = (4, 10).into();
        display(m, p);
        assert!(!m.resolve(p).unwrap())
    }

    #[test]
    fn donut_hit() {
        let m = Mechanic::Donut { pos: (10, 10).into(), size: 10, inner_size: 3 };
        let p: Point = (4, 10).into();
        display(m, p);
        assert!(m.resolve(p).unwrap())
    }

    #[test]
    fn donut_miss1() {
        let m = Mechanic::Donut { pos: (10, 10).into(), size: 10, inner_size: 3 };
        let p: Point = (8, 9).into();
        display(m, p);
        assert!(!m.resolve(p).unwrap())
    }

    #[test]
    fn donut_miss2() {
        let m = Mechanic::Donut { pos: (10, 10).into(), size: 10, inner_size: 3 };
        let p: Point = (0, 0).into();
        display(m, p);
        assert!(!m.resolve(p).unwrap())
    }

    #[test]
    fn laser_hit1() {
        let m = Mechanic::Laser { orientation: 0., offsetx: 6., offsety: 3., thickness: 3. };
        let p: Point = (8, 11).into();
        display(m, p);
        assert!(m.resolve(p).unwrap())
    }
    
    #[test]
    fn laser_hit2() {
        let m = Mechanic::Laser { orientation: PI*0.5, offsetx: 3., offsety: 15., thickness: 1. };
        let p: Point = (4, 15).into();
        display(m, p);
        assert!(m.resolve(p).unwrap())
    }

    #[test]
    fn laser_hit3() {
        let m = Mechanic::Laser { orientation: PI*0.5, offsetx: 3., offsety: 15., thickness: 2. };
        let p: Point = (4, 14).into();
        display(m, p);
        assert!(m.resolve(p).unwrap())
    }

    #[test]
    fn laser_hit4() {
        let m = Mechanic::Laser { orientation: PI*0.4, offsetx: 3., offsety: 15., thickness: 2. };
        let p: Point = (26, 6).into();
        display(m, p);
        assert!(m.resolve(p).unwrap())
    }

    #[test]
    fn laser_hit5() {
        let m = Mechanic::Laser { orientation: PI*0.15, offsetx: 7., offsety: 17., thickness: 2. };
        let p: Point = (16, 1).into();
        display(m, p);
        assert!(m.resolve(p).unwrap())
    }

    #[test]
    fn laser_miss1() {
        let m = Mechanic::Laser { orientation: PI*0.1, offsetx: 6., offsety: 3., thickness: 6. };
        let p: Point = (11, 11).into();
        display(m, p);
        assert!(!m.resolve(p).unwrap())
    }

    #[test]
    fn laser_miss2() {
        let m = Mechanic::Laser { orientation: PI*0.9, offsetx: 6., offsety: 7., thickness: 5. };
        let p: Point = (4, 20).into();
        display(m, p);
        assert!(!m.resolve(p).unwrap())
    }
    
    #[test]
    fn laser_miss3() {
        let m = Mechanic::Laser { orientation: PI*1.2, offsetx: 20., offsety: 8., thickness: 7. };
        let p: Point = (1, 20).into();
        display(m, p);
        assert!(!m.resolve(p).unwrap())
    }

    #[test]
    fn cleave_hit1() {
        let m = Mechanic::Cleave { orientation: 0., start: 0, end: 24 };
        let p: Point = (20, 15).into();
        display(m, p);
        assert!(m.resolve(p).unwrap())
    }

    #[test]
    fn cleave_hit2() {
        let m = Mechanic::Cleave { orientation: 0., start: 2, end: 4 };
        let p: Point = (3, 15).into();
        display(m, p);
        assert!(m.resolve(p).unwrap())
    }

    #[test]
    fn cleave_hit3() {
        let m = Mechanic::Cleave { orientation: PI*0.5, start: 10, end: 12 };
        let p: Point = (4, 10).into();
        display(m, p);
        assert!(m.resolve(p).unwrap())
    }

    #[test]
    fn cleave_hit4() {
        let m = Mechanic::Cleave { orientation: PI*0.25, start: 10, end: 12 };
        let p: Point = (13, 10).into();
        display(m, p);
        assert!(m.resolve(p).unwrap())
    }

    #[test]
    fn cleave_miss1() {
        let m = Mechanic::Cleave { orientation: 0., start: 0, end: 28 };
        let p: Point = (30, 10).into();
        display(m, p);
        assert!(!m.resolve(p).unwrap())
    }

    #[test]
    fn cleave_miss2() {
        let m = Mechanic::Cleave { orientation: PI*0.5, start: 32, end: 6 };
        let p: Point = (13, 3).into();
        display(m, p);
        assert!(!m.resolve(p).unwrap())
    }

    #[test]
    fn cleave_miss3() {
        let m = Mechanic::Cleave { orientation: PI*0.4, start: 4, end: 35 };
        let p: Point = (5, 3).into();
        display(m, p);
        assert!(!m.resolve(p).unwrap())
    }

    #[test]
    fn gaze_miss_sw_e() {
        let m = Mechanic::Gaze { pos: (10, 10).into() };
        let p = Point::new(15, 15);
        let mut game = Game::new(((0, 0).into(), (32, 32).into()), 5);
        game.player.force_move_to(p);
        game.player.force_set_direction(Direction::E);
        display(m, p);
        game.mechanic_queue.push_front(MechanicTimer::new(m, 0));
        game.advance().unwrap();
        assert!(game.player.is_alive());
    }

    #[test]
    fn gaze_miss_sw_n() {
        let m = Mechanic::Gaze { pos: (10, 10).into() };
        let p = Point::new(15, 15);
        let mut game = Game::new(((0, 0).into(), (32, 32).into()), 5);
        game.player.force_move_to(p);
        game.player.force_set_direction(Direction::N);
        display(m, p);
        game.mechanic_queue.push_front(MechanicTimer::new(m, 0));
        game.advance().unwrap();
        assert!(game.player.is_alive());
    }

    #[test]
    fn gaze_miss_se_w() {
        let m = Mechanic::Gaze { pos: (25, 10).into() };
        let p = Point::new(15, 15);
        let mut game = Game::new(((0, 0).into(), (32, 32).into()), 5);
        game.player.force_move_to(p);
        game.player.force_set_direction(Direction::W);
        display(m, p);
        game.mechanic_queue.push_front(MechanicTimer::new(m, 0));
        game.advance().unwrap();
        assert!(game.player.is_alive());
    }

    #[test]
    fn gaze_miss_se_n() {
        let m = Mechanic::Gaze { pos: (25, 10).into() };
        let p = Point::new(15, 15);
        let mut game = Game::new(((0, 0).into(), (32, 32).into()), 5);
        game.player.force_move_to(p);
        game.player.force_set_direction(Direction::N);
        display(m, p);
        game.mechanic_queue.push_front(MechanicTimer::new(m, 0));
        game.advance().unwrap();
        assert!(game.player.is_alive());
    }

    #[test]
    fn gaze_miss_nw_s() {
        let m = Mechanic::Gaze { pos: (10, 25).into() };
        let p = Point::new(15, 15);
        let mut game = Game::new(((0, 0).into(), (32, 32).into()), 5);
        game.player.force_move_to(p);
        game.player.force_set_direction(Direction::S);
        display(m, p);
        game.mechanic_queue.push_front(MechanicTimer::new(m, 0));
        game.advance().unwrap();
        assert!(game.player.is_alive());
    }

    #[test]
    fn gaze_miss_nw_e() {
        let m = Mechanic::Gaze { pos: (10, 25).into() };
        let p = Point::new(15, 15);
        let mut game = Game::new(((0, 0).into(), (32, 32).into()), 5);
        game.player.force_move_to(p);
        // game.player.force_move_to();
        game.player.force_set_direction(Direction::E);
        display(m, p);
        game.mechanic_queue.push_front(MechanicTimer::new(m, 0));
        game.advance().unwrap();
        assert!(game.player.is_alive());
    }

    #[test]
    fn gaze_miss_ne_w() {
        let m = Mechanic::Gaze { pos: (25, 25).into() };
        let p = Point::new(15, 15);
        let mut game = Game::new(((0, 0).into(), (32, 32).into()), 5);
        game.player.force_move_to(p);
        game.player.force_set_direction(Direction::W);
        display(m, p);
        game.mechanic_queue.push_front(MechanicTimer::new(m, 0));
        game.advance().unwrap();
        assert!(game.player.is_alive());
    }

    #[test]
    fn gaze_hit1() {
        let m = Mechanic::Gaze { pos: (10, 10).into() };
        let p = Point::new(15, 15);
        let mut game = Game::new(((0, 0).into(), (32, 32).into()), 5);
        game.player.force_move_to(p);
        game.player.force_set_direction(Direction::W);
        display(m, p);
        game.mechanic_queue.push_front(MechanicTimer::new(m, 0));
        game.advance().unwrap();
        assert!(!game.player.is_alive());
    }

    #[test]
    fn gaze_hit2() {
        let m = Mechanic::Gaze { pos: (10, 10).into() };
        let p = Point::new(15, 15);
        let mut game = Game::new(((0, 0).into(), (32, 32).into()), 5);
        game.player.force_move_to(p);
        game.player.force_set_direction(Direction::S);
        display(m, p);
        game.mechanic_queue.push_front(MechanicTimer::new(m, 0));
        game.advance().unwrap();
        assert!(!game.player.is_alive());
    }

    #[test]
    fn gaze_hit_nw_w() {
        let m = Mechanic::Gaze { pos: (10, 25).into() };
        let p = Point::new(15, 15);
        let mut game = Game::new(((0, 0).into(), (32, 32).into()), 5);
        game.player.force_move_to(p);
        game.player.force_set_direction(Direction::W);
        display(m, p);
        game.mechanic_queue.push_front(MechanicTimer::new(m, 0));
        game.advance().unwrap();
        assert!(!game.player.is_alive());
    }

    #[test]
    fn gaze_hit_nw_n() {
        let m = Mechanic::Gaze { pos: (10, 25).into() };
        let p = Point::new(15, 15);
        let mut game = Game::new(((0, 0).into(), (32, 32).into()), 5);
        game.player.force_move_to(p);
        game.player.force_set_direction(Direction::N);
        display(m, p);
        game.mechanic_queue.push_front(MechanicTimer::new(m, 0));
        game.advance().unwrap();
        assert!(!game.player.is_alive());
    }

    #[test]
    fn gaze_hit_ne_w() {
        let m = Mechanic::Gaze { pos: (25, 25).into() };
        let p = Point::new(15, 15);
        let mut game = Game::new(((0, 0).into(), (32, 32).into()), 5);
        game.player.force_move_to(p);
        game.player.force_set_direction(Direction::E);
        display(m, p);
        game.mechanic_queue.push_front(MechanicTimer::new(m, 0));
        game.advance().unwrap();
        assert!(!game.player.is_alive());
    }

    #[test]
    fn puddle_miss() {
        let m = Mechanic::Puddle { pos: (10, 10).into(), size: 5, duration: 2 };
        let p = Point::new(15, 15);
        let mut game = Game::new(((0, 0).into(), (32, 32).into()), 5);
        game.player.force_move_to(p);
        display(m, p);
        game.mechanic_queue.push_front(MechanicTimer::new(m, 0));
        game.advance().unwrap();
        game.advance().unwrap();
        game.player.force_move_to((10, 10).into());
        game.advance().unwrap();
        assert!(game.player.is_alive());
    }

    #[test]
    fn puddle_hit() {
        let m = Mechanic::Puddle { pos: (10, 10).into(), size: 5, duration: 3 };
        let p = Point::new(15, 15);
        let mut game = Game::new(((0, 0).into(), (32, 32).into()), 5);
        game.player.force_move_to(p);
        display(m, p);
        game.mechanic_queue.push_front(MechanicTimer::new(m, 0));
        game.advance().unwrap();
        game.advance().unwrap();
        game.player.force_move_to((10, 10).into());
        game.advance().unwrap();
        assert!(!game.player.is_alive());
    }
}
