module Commands exposing (..)

import CommandHandler
import Commands.ListWads as ListWads
import Game exposing (Game)


type Command
    = ListWads ListWads.State


handleLine : String -> Maybe Command -> Game -> CommandHandler.Result ListWads.State
handleLine line commandState game =
    case commandState of
        Just (ListWads state) ->
            ListWads.handle line game state

        _ ->
            CommandHandler.Nothing
