use std::error::Error;
use std::f32::consts::PI;
use std::io::{stdin, stdout, Write};

fn main() {
    loop {
        match parse_one() {
            Err(s) => println!("{:#?}", s),
            _ => ()
        }
    }
}

fn parse_one() -> Result<(), Box<dyn Error>>{
    let mut input = String::new();
    print!("deg: ");
    stdout().flush().unwrap();
    stdin().read_line(&mut input)?;
    
    let deg: f32 = input.trim().parse()?;
    println!("rad: {}", deg * PI / 180.);
    Ok(())
}
