use "lib.sml";

fun compute s mapL=
    let
	fun EXP nil = raise SyntaxError
          | EXP (h::t) =
            if isInt h then (toInt h, t)
	    else if h ="(" then
		if hd t =  "fibo" orelse hd t ="fact" then FUNC(h::t)
		else if isOpr(hd t) then COMP(h::t)
		else raise SyntaxError
	    else if isAlp h then (findValue h mapL,t)
            else raise SyntaxError

	and COMP nil = raise SyntaxError
          | COMP (h::t) =
            if h = "+" then
		let
		    val (v1,t1) = EXP t
		    val (v2,t2) = EXP t1
		in
		    (v1 + v2,t2)
		end
	    else if h = "*" then
		let
		    val (v1,t1) = EXP t
		    val (v2,t2) = EXP t1
		in
		    (v1 * v2,t2)
		end
	    else if h = "-" then
		let
		    val (v1,t1) = EXP t
		    val (v2,t2) = EXP t1
		in
		    (v1 - v2,t2)
		end

	    else if h = "/" then
		let
		    val (v1,t1) = EXP t
		    val (v2,t2) = EXP t1
		in
		    (v1 div v2,t2)
		end

	    else if h = "(" then
		let
		    val (v1,t1) = COMP t
		    val (v2,t2) = COMP t1
		in
		    (v1,t2)
		end
	    else if h = ")" then (0,t)
            else raise SyntaxError
	and FUNC nil = raise SyntaxError
	  | FUNC (h::t) =
	    if h = "fibo" then
		let
		   val (v1,t1) = EXP t
		in
		     (fibo v1,t1)
		end
	    else if h = "fact" then
		let
		    val (v1,t1) = EXP t
		in
		    (fact v1,t1)
		end
	    else if h = "(" then
		let
		    val (v1,t1) = FUNC t
		    val (v2,t2) = FUNC t1
		in
		    (v1,t2)
		end
	    else if h = ")" then (1,t)
	    else raise SyntaxError
	and findValue s nil = raise NotDefined
	  | findValue s ((h:string*int)::t) =
	    if s = #1(h)
	    then #2(h)
	    else findValue s t		    
    in
	let
            val (result,rest) = EXP (separate s)
	in
            if rest = nil then result else raise SyntaxError
	end
    end;

  
  
