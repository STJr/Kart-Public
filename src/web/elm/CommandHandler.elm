module CommandHandler exposing (..)

import Game exposing (Game)


type Result state
    = Finished Game
    | Pending state
    | Nothing
    | Error
