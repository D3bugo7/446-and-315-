(* parsing util functions *)
let is_lower_case c = 'a' <= c && c <= 'z'
let is_upper_case c = 'A' <= c && c <= 'Z'
let is_alpha c = is_lower_case c || is_upper_case c
let is_digit c = '0' <= c && c <= '9'
let is_alphanum c = is_lower_case c || is_upper_case c || is_digit c
let is_blank c = String.contains " \012\n\r\t" c
let explode s = List.of_seq (String.to_seq s)
let implode ls = String.of_seq (List.to_seq ls)
let readlines (file : string) : string =
  let fp = open_in file in
  let rec loop () =
    match input_line fp with
    | s -> s ^ "\n" ^ loop ()
    | exception End_of_file -> ""
  in
  let res = loop () in
  let () = close_in fp in
  res

(* end of util functions *)

(* parser combinators *)
type 'a parser = char list -> ('a * char list) option

let parse (p : 'a parser) (s : string) : ('a * char list) option = p (explode s)
let pure (x : 'a) : 'a parser = fun ls -> Some (x, ls)
let fail : 'a parser = fun ls -> None
let bind (p : 'a parser) (q : 'a -> 'b parser) : 'b parser =
  fun ls ->
  match p ls with
  | Some (a, ls) -> q a ls
  | None -> None
let ( >>= ) = bind
let ( let* ) = bind
let read : char parser =
  fun ls ->
  match ls with
  | x :: ls -> Some (x, ls)
  | _ -> None
let satisfy (f : char -> bool) : char parser =
  fun ls ->
  match ls with
  | x :: ls -> if f x then Some (x, ls) else None
  | _ -> None
let char (c : char) : char parser = satisfy (fun x -> x = c)
let seq (p1 : 'a parser) (p2 : 'b parser) : 'b parser =
  fun ls ->
  match p1 ls with
  | Some (_, ls) -> p2 ls
  | None -> None
let ( >> ) = seq
let seq' (p1 : 'a parser) (p2 : 'b parser) : 'a parser =
  fun ls ->
  match p1 ls with
  | Some (x, ls) -> (
      match p2 ls with
      | Some (_, ls) -> Some (x, ls)
      | None -> None)
  | None -> None
let ( << ) = seq'
let alt (p1 : 'a parser) (p2 : 'a parser) : 'a parser =
  fun ls ->
  match p1 ls with
  | Some (x, ls) -> Some (x, ls)
  | None -> p2 ls
let ( <|> ) = alt
let map (p : 'a parser) (f : 'a -> 'b) : 'b parser =
  fun ls ->
  match p ls with
  | Some (a, ls) -> Some (f a, ls)
  | None -> None
let ( >|= ) = map
let ( >| ) p c = map p (fun _ -> c)
let rec many (p : 'a parser) : 'a list parser =
  fun ls ->
  match p ls with
  | Some (x, ls) -> (
      match many p ls with
      | Some (xs, ls) -> Some (x :: xs, ls)
      | None -> Some ([ x ], ls))
  | None -> Some ([], ls)
let rec many1 (p : 'a parser) : 'a list parser =
  fun ls ->
  match p ls with
  | Some (x, ls) -> (
      match many p ls with
      | Some (xs, ls) -> Some (x :: xs, ls)
      | None -> Some ([ x ], ls))
  | None -> None
let rec many' (p : unit -> 'a parser) : 'a list parser =
  fun ls ->
  match p () ls with
  | Some (x, ls) -> (
      match many' p ls with
      | Some (xs, ls) -> Some (x :: xs, ls)
      | None -> Some ([ x ], ls))
  | None -> Some ([], ls)
let rec many1' (p : unit -> 'a parser) : 'a list parser =
  fun ls ->
  match p () ls with
  | Some (x, ls) -> (
      match many' p ls with
      | Some (xs, ls) -> Some (x :: xs, ls)
      | None -> Some ([ x ], ls))
  | None -> None
let whitespace : unit parser =
  fun ls ->
  match ls with
  | c :: ls -> if String.contains " \012\n\r\t" c then Some ((), ls) else None
  | _ -> None
let ws : unit parser = many whitespace >| ()
let ws1 : unit parser = many1 whitespace >| ()
let digit : char parser = satisfy is_digit
let natural : int parser =
  fun ls ->
  match many1 digit ls with
  | Some (xs, ls) -> Some (int_of_string (implode xs), ls)
  | _ -> None
let literal (s : string) : unit parser =
  fun ls ->
  let cs = explode s in
  let rec loop cs ls =
    match (cs, ls) with
    | [], _ -> Some ((), ls)
    | c :: cs, x :: xs -> if x = c then loop cs xs else None
    | _ -> None
  in
  loop cs ls
let keyword (s : string) : unit parser = literal s >> ws >| ()

(* end of parser combinators *)

(* Grammar *)
type value = VInt of int | VBool of bool | VUnit
type command =
  | Push of value
  | Pop of int
  | Trace of int
  | Add of int
  | Sub of int
  | Mul of int
  | Div of int
(*end of grammar*)
(* Parsers *)
let parse_int : value parser = natural >|= fun n -> VInt n
let parse_bool_true : value parser = keyword "True" >|= fun () -> VBool true
let parse_bool_false : value parser = keyword "False" >|= fun () -> VBool false
let parse_unit : value parser = keyword "()" >|= fun () -> VUnit
let parse_const : value parser = parse_int <|> parse_bool_true <|> parse_bool_false <|> parse_unit

let parse_push : command parser =
  keyword "Push" >>= fun () ->
  parse_const >|= fun v -> Push v

let parse_pop : command parser =
  keyword "Pop" >>= fun () ->
  natural >|= fun n -> Pop n

let parse_trace : command parser =
  keyword "Trace" >>= fun () ->
  natural >|= fun n -> Trace n

let parse_add : command parser =
  keyword "Add" >>= fun () ->
  natural >|= fun n -> Add n

let parse_sub : command parser =
  keyword "Sub" >>= fun () ->
  natural >|= fun n -> Sub n

let parse_mul : command parser =
  keyword "Mul" >>= fun () ->
  natural >|= fun n -> Mul n

let parse_div : command parser =
  keyword "Div" >>= fun () ->
  natural >|= fun n -> Div n

let parse_com : command parser =
  parse_push <|> parse_pop <|> parse_trace <|> parse_add <|> parse_sub <|> parse_mul <|> parse_div

let parse_coms : command list parser = many (parse_com << ws)

(*this version doesn't use loops or mutable variables*)
let string_of_value = function
  | VInt n -> string_of_int n
  | VBool b -> string_of_bool b
  | VUnit -> "()"
  
  let evaluate_commands (cmds: command list) : string list =
    let rec aux cmds stack log =
      match cmds with
      | [] -> List.rev log
      | Push v :: rest -> aux rest (v :: stack) log
      | Pop n :: rest ->
          if n < 0 || List.length stack < n then ["Error"]
          else aux rest (List.drop n stack) log (*list.drop needs to be implemented or pop logic needs to change*)
      | Trace n :: rest ->
          if n < 0 || List.length stack < n then ["Error"]
          else
            let top_n = List.take n stack in
            aux rest (List.drop n stack) (List.rev_append (List.map string_of_value top_n) log)
      | Add n :: rest ->
          (* Logic for adding n top-most elements *)
          if n < 0 || List.length stack < n then ["Error"]
          else
            let top_n = List.take n stack in
            if List.for_all (function VInt _ -> true | _ -> false) top_n then
              let sum = List.fold_left (fun acc (VInt x) -> acc + x) 0 top_n in
              aux rest (VInt sum :: List.drop n stack) log
            else ["Error"]
      (* implement logic for Sub, Mul, Div *)
      | _ -> ["Error"] (* Catch-all for unexpected cases *)
    in
    aux cmds [] []  (* Initialize with empty stack and log *)

    let interp (src : string) : string list =
      match parse parse_coms src with
      | Some (commands, _) -> evaluate_commands commands
      | None -> ["Error"]

(* End of version *)

(*(* Interpreter *)
let string_of_value = function
  | VInt n -> string_of_int n
  | VBool b -> string_of_bool b
  | VUnit -> "()"

let interp (src : string) : string list =
  match parse parse_coms src with
  | Some (commands, _) ->
    let stack = ref [] in
    let log = ref [] in
    let error = ref false in
    List.iter
      (fun command ->
        if !error then () else
          match command with
          | Push v -> stack := v :: !stack
          | Pop n ->
            if n < 0 || List.length !stack < n then error := true
            else stack := List.drop n !stack
          | Trace n ->
            if n < 0 || List.length !stack < n then error := true
            else
              let top_n = List.take n !stack in
              stack := List.drop n !stack;
              log := List.rev_append (List.map string_of_value top_n) !log
          | Add n ->
            if n < 0 || List.length !stack < n then error := true
            else
              let top_n = List.take n !stack in
              if not (List.for_all (function VInt _ -> true | _ -> false) top_n) then
                error := true
              else
                let sum = List.fold_left (fun acc v -> match v with VInt x -> acc + x | _ -> acc) 0 top_n in
                stack := VInt sum :: !stack
          | Sub n ->
            if n < 0 || List.length !stack < n then error := true
            else
              let top_n = List.take n !stack in
              if not (List.for_all (function VInt _ -> true | _ -> false) top_n) then
                error := true
              else
                match top_n with
                | head :: rest ->
                  let sum_rest = List.fold_left (fun acc v -> match v with VInt x -> acc + x | _ -> acc) 0 rest in
                  (match head with VInt h -> stack := VInt (h - sum_rest) :: !stack | _ -> error := true)
                | [] -> stack := VInt 0 :: !stack (* n = 0 case *)
          | Mul n ->
            if n < 0 || List.length !stack < n then error := true
            else
              let top_n = List.take n !stack in
              if not (List.for_all (function VInt _ -> true | _ -> false) top_n) then
                error := true
              else
                let product = List.fold_left (fun acc v -> match v with VInt x -> acc * x | _ -> acc) 1 top_n in
                stack := VInt product :: !stack
          | Div n ->
            if n < 0 || List.length !stack < n then error := true
            else
              let top_n = List.take n !stack in
              if not (List.for_all (function VInt _ -> true | _ -> false) top_n) then
                error := true
              else
                match top_n with
                | head :: rest ->
                  let product_rest = List.fold_left (fun acc v -> match v with VInt x -> acc * x | _ -> acc) 1 rest in
                  if product_rest = 0 then error := true
                  else (match head with VInt h -> stack := VInt (h / product_rest) :: !stack | _ -> error := true)
                | [] -> stack := VInt 1 :: !stack (* n = 0 case *))
      commands;
    if !error then ["Error"] else List.rev !log
  | None -> ["Error"]

(* Calling (main "test.txt") will read the file test.txt and run interp on it.
   This is only used for debugging and will not be used by the gradescope
   autograder.
*)
let main fname =
  let src = readlines fname in
  interp src
*)