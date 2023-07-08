module Model exposing (Model)

import Commands exposing (Command)
import Game exposing (Game)
import Status exposing (Status)


type alias Model =
    { outputLines : List String
    , emStatus : Status
    , helpShown : Bool
    , game : Game
    , commandState : Maybe Command
    }
