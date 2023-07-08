module Commands.ListWads exposing (State, handle, init)

import CommandHandler
import Game exposing (Game)
import List exposing (length)
import Parser exposing ((|.), (|=), chompUntilEndOr, end, int, keyword, map, number, oneOf, run, spaces, succeed, symbol)


type alias State =
    { expectedLines : Maybe Int
    , wads : List String
    }


type ListWadsLine
    = Count Int
    | Wad String


init : State
init =
    { expectedLines = Nothing, wads = [] }


parseLine : String -> Result (List Parser.DeadEnd) ListWadsLine
parseLine =
    let
        -- There are 7 wads loaded:
        countParser : Parser.Parser ListWadsLine
        countParser =
            succeed Count
                |. keyword "There"
                |. spaces
                |. keyword "are"
                |. spaces
                |= int
                |. spaces
                |. keyword "wads"
                |. spaces
                |. keyword "loaded"
                |. symbol ":"
                |. end

        --    04: maps.kart index.js:53:13
        wadParser : Parser.Parser ListWadsLine
        wadParser =
            map Wad <|
                Parser.getChompedString <|
                    succeed ()
                        |. spaces
                        |. oneOf
                            [ succeed ()
                                |. number
                                    { int = Just (always ())
                                    , float = Just (always ())
                                    , hex = Nothing
                                    , octal = Just (always ())
                                    , binary = Nothing
                                    }
                                |. number
                                    { int = Just (always ())
                                    , float = Just (always ())
                                    , hex = Nothing
                                    , octal = Just (always ())
                                    , binary = Nothing
                                    }
                            , succeed () |. keyword "IWAD"
                            ]
                        |. symbol ":"
                        |. spaces
                        |. chompUntilEndOr "\n"
    in
    run <|
        oneOf
            [ countParser
            , wadParser
            ]


handle : String -> Game -> State -> CommandHandler.Result State
handle line game state =
    case parseLine line of
        Ok parsedLine ->
            case parsedLine of
                Count count ->
                    CommandHandler.Pending { state | expectedLines = Just count }

                Wad wadName ->
                    case state.expectedLines of
                        Just count ->
                            if length state.wads == count - 1 then
                                CommandHandler.Finished <|
                                    (game
                                        |> Game.setAddons (wadName :: state.wads)
                                    )

                            else
                                CommandHandler.Pending { state | wads = wadName :: state.wads }

                        Nothing ->
                            CommandHandler.Pending state

        Err _ ->
            CommandHandler.Error
