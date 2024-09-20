# Introduction
This project implements a simple chess game. You can play the game by runing two terminals and executing `server` and `client` separately.

## Environment
This project is supposed to be runing on Linux/Unix environment.

## Compile
If instailed `makefile`, run either `$make` or `$make all`, it will create a `/play` directory which contains the executable files. `$make clean` will remove the `/play` directory.

If you don't have `makefile`, you can also use `gcc` to manually compile the code as following:

```bash
$cd src
$mkdir -p ./play
$gcc -c Game.c -o Game.o
$gcc -c Client.c -o Client.o
$gcc -o ../play/client Game.o Client.o
$gcc -c Server.c -o Server.o
$gcc -o ../play/server Game.o Server.o
$rm -f Game.o Client.o Server.o
```

:anguished:If you found `Resources.h: No such file or directory`, you can replace `#include "Resource.h"` in each `.c` files using `#include "../include/Resource.h"` 

## Usage
In a terminal, first run `$play/server`, and it will be waiting for the connection of client site. In anohter terminal, run `$play/client`, it will connect to the server site. 

The client site will be the starter of the game and use white pieces.

Below is showing the start state of the game:
```
  a b c d e f g h
8 r n b q k b n r 8
7 p p p p p p p p 7
6 . . . . . . . . 6
5 . . . . . . . . 5
4 . . . . . . . . 4
3 . . . . . . . . 3
2 P P P P P P P P 2
1 R N B Q K B N R 1
  a b c d e f g h
```
The letters on the top and bottom rows, and the numbers on the sides represent the location. In the above example, `a2` represents `P`.

The `.` represents an empty space. The upper letters represent white pieces, and lower letters are black pieces.

- `P` -> Pawn
- `R` -> Rook
- `N` -> Knight
- `B` -> Bishop
- `Q` -> Queen
- `K` -> King

## Instructions
In this program you will send instructions to complete the data interation. 

:exclamation:Note that each instruction must start with a `/`.

#### Display a chess board
```
/chessboard
```
This will display the state of current game.

#### Move a piece
```
/move a2a3
```
This will help you move the piece from `a2` to `a3`.

You may use the following instruction for promotion of a `pawn`:
```
/move a7a8b
```
This means move a `pawn` form `a7` to `a8` and promote to a `bishop`.

#### Terminate game
```
/forfeit
```
Use this instruction to terminate the game.

#### Import a game using FEN
```
/import rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b
```
The structure of FEN will be talking in `FEN string` section. 
This instruction will update this game to a new state.

#### Load an existing game
```
/load Junjie 2
```
This will look for a database file and update the state of current game.

The 2nd argument is a username of who stored the game state. It should not contain white spaces. 

The 3rd argument is a `saving number` representing the number of game state. For exmaple, if there are 10 game states storing in the database, the above instruction will load the 2nd game.

#### Save game state
```
/save Junjie
```
This will storage the current game state in database using a username.

## Summary instruction table
| **Instruction** | **Parameter requirement** | **Example** | **Description** |
|:-------|:-------|:---|:-----------|
|`/chessboard`| `None` | `/chessboard` | Display the state of game |
| `/move` |Start and end location| `/move a2a3` | Move a piece |
|`/forfeit`| `None` | `/forfeit` | Terminate game |
| `/import` | FEN string | `/import rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b` | Update current game state |
| `/load` | `Username` and `saving number` | `/load Junjie 2` | Load existing game state |
| `/save` | `username` | `/save Junjie` | Save game state in database |

:scream:**Note:**
In the source code you may find a `/none` instruction. This is used to switch the controller when `load` a game state that has a different controller. It's not supposed to be used as a regular instruction during gaming.

## FEN string
A FEN string stores the state of game in a specific format. It is used to store the game in database, or pass as an argument for `/import` instruction.

Example:
```
rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b
```

Each element separated by a  `/` represents a row of the game.

The number represents number of spaces in a row.

The letter in the end separating by a white space represents the current player (`b` for black pieces, `w` for white pieces).

The game state of the above example will look like:
```
  a b c d e f g h
8 r n b q k b n r 8
7 p p . p p p p p 7
6 . . . . . . . . 6
5 . . p . . . . . 5
4 . . . . P . . . 4
3 . . . . . N . . 3
2 P P P P . P P P 2
1 R N B Q K B . R 1
  a b c d e f g h
```

## Project structures
There are 2 directories `include` and `src` in this project. In `src`, there is a special file `game_database.txt` that stores game states. 

:dizzy_face:The program might not be able to run if removed this file.

## Insufficient
Some complex rulings such as en passant, castling, checking, and checkmating are excluded.


## Authors
- Implement: [@Junjie Huang](https://github.com/JunjieHuang2002)
- Architecture: [@Kevin McDonnell](https://sites.google.com/cs.stonybrook.edu/ktm/home)

## License
[MIT](https://choosealicense.com/licenses/mit/)
