use std::collections::VecDeque;

use crate::{player::Player, Direction, Mechanic, Point, FLARE_MODIFIER, KB_AMOUNT, TOWER_DAMAGE};

#[cfg(feature = "serialize")]
use serde::Serialize;
#[cfg(feature = "deserialize")]
use serde::Deserialize;

#[cfg_attr(feature = "debug", derive(Debug))]
#[cfg_attr(feature = "serialize", derive(Serialize))]
#[cfg_attr(feature = "deserialize", derive(Deserialize))]
#[derive(Clone, Copy)]
pub struct MechanicTimer {
    #[cfg_attr(any(feature = "serialize", feature = "deserialize"), serde(flatten))]
    pub mechanic: Mechanic,
    pub timer: usize,
}

impl MechanicTimer {
    pub fn new(mechanic: Mechanic, timer: usize) -> MechanicTimer {
        MechanicTimer { mechanic, timer }
    }
}

#[cfg_attr(feature = "debug", derive(Debug))]
pub struct Game {
    pub tick: usize,
    pub arena_bounds: (Point, Point),
    pub mechanic_queue: VecDeque<MechanicTimer>,
    pub player: Player,
    pub boss_health: u32,
}

#[cfg_attr(feature = "debug", derive(Debug))]
pub enum GameState {
    Running,
    Victory,
    Defeat,
}

impl Game {
    /// Creates a new [`Game`]
    #[inline]
    pub fn new(arena_bounds: (Point, Point), boss_health: u32) -> Game {
        Game {
            tick: 0,
            arena_bounds,
            mechanic_queue: VecDeque::new(),
            player: Player::new(),
            boss_health,
        }
    }

    #[inline]
    fn do_gaze(&self, pos: Point) -> bool {
        match (self.player.last_moved(), (self.player.pos() - pos).signum()) {
            (Direction::N, Point { x: _, y: -1 }) => true,
            (Direction::S, Point { x: _, y: 1 }) => true,
            (Direction::E, Point { x: -1, y: _ }) => true,
            (Direction::W, Point { x: 1, y: _ }) => true,
            _ => false,
        }
    }

    pub fn is_player_in_bounds(&self) -> bool {
        let player_pos = self.player.pos();
        let leftlower = self.arena_bounds.0;
        let rightupper = self.arena_bounds.1;

        !(player_pos.x < leftlower.x
            || player_pos.x > rightupper.x
            || player_pos.y < leftlower.y
            || player_pos.y > rightupper.y)
    }

    /// Advances a tick, resolving mechanics and applying damage to [`Game::player`]
    pub fn advance(&mut self) -> Result<(), &str> {
        match self.check_state() {
            GameState::Running => (),
            _ => return Err("Invalid state!"),
        }
        let mut push_after: Vec<MechanicTimer> = Vec::with_capacity(3);
        while let Some(mut mtimer) = self.mechanic_queue.pop_front() {
            if mtimer.timer > self.tick {
                self.mechanic_queue.push_front(mtimer);
                break;
            }

            // This implementation doesn't actually check if you stepped
            // into a puddle, just if you ended your turn in it
            match mtimer.mechanic {
                Mechanic::Puddle { pos, size, duration } => {
                    if duration < 1 {
                        continue;
                    }
                    mtimer.mechanic = Mechanic::Puddle {
                        pos,
                        size,
                        duration: duration - 1,
                    };
                    push_after.push(mtimer);
                }
                _ => ()
            }

            if !mtimer.mechanic.resolve(self.player.pos())? {
                continue;
            }

            match mtimer.mechanic {
                Mechanic::Tower { pos } => self.player.damage(
                    (TOWER_DAMAGE as f32 * (1. + pos.dist(self.player.pos()).unwrap_or(100.)))
                        as i32,
                ),
                Mechanic::Flare { pos } => self.player.damage(
                    ((3. / ((pos.dist(self.player.pos())).unwrap_or(i32::MAX as f32) + 2.5)
                        .log10()
                        .powf(5.))
                        * FLARE_MODIFIER) as i32,
                ), // ((0f32).log2()) as i32 == -2147483648
                Mechanic::Wind { orientation } => self.player.force_move_by(
                    (
                        (orientation.cos()
                            * KB_AMOUNT)
                            .round() as i32,
                        (orientation.sin()
                            * KB_AMOUNT)
                            .round() as i32,
                    )
                        .into(),
                ),
                // Mechanic::Knockback { pos } => todo!(),
                Mechanic::Gaze { pos } => {
                    if self.do_gaze(pos) {
                        self.player.kill();
                    }
                }
                Mechanic::Raidwide { damage } => self.player.damage(damage as i32),
                Mechanic::Pyretic => self
                    .player
                    .damage(TOWER_DAMAGE * self.player.did_move() as i32),
                _ => self.player.kill(),
            }
        }
        for mt in push_after {
            self.mechanic_queue.push_front(mt);
        }
        self.tick += 1;

        self.player.tick();
        if !self.is_player_in_bounds() {
            self.player.kill();
        }
        Ok(())
    }

    pub fn check_state(&self) -> GameState {
        if !self.player.is_alive() {
            return GameState::Defeat;
        }
        if self.boss_health == 0 {
            return GameState::Victory;
        }

        GameState::Running
    }

    /// Damages the boss by 1
    #[inline]
    pub fn damage_boss(&mut self) {
        self.boss_health -= 1
    }
}
