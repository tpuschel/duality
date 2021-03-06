# Core - The Duality Core Calculus


**!!! Currently outdated and in the process of being rewitten !!!**

The files in this folder implement the Duality Core Calculus (DCC),
and depend on the files in the 'support' folder.

While DCC has an informal syntactic representation (just to be able to write it in text documents),
the actual syntax of Duality is implemented and described in the 'syntax' folder.

DCC is a typed lambda calculus featuring, among other things, dependent types, subtyping, gradual typing,
unrestricted recursion and full type inference.

Novel language features include the axiomatization of syntactical equality as a form of mapping, and the reformulation of traditional branching constructs as ones acting upon the inherent, ternary information of eliminations.

To fully understand the whys and hows of DCC, it's best to follow the design evolution
of a simple base calculus, eventually culminating in DCC.

## Humble beginnings

Our starting calculus is a very simple lambda calculus with type annotations on bindings:

Expression :=
- (Abstraction) `λ (v : e1) -> e2`
- (Application) `e1 e2`
- (Variable) &nbsp;&nbsp;&nbsp;&nbsp;&nbsp; `v`

E1 and e2 are expressions.

What's crucial to note here is that the type annotation in the abstraction can be *any arbitrary expression*.
This eliminates any distinction between a 'term language' and a 'type language'.

Thus, **the term and type languages are unified.**

Let's see what consequences that bears by examining how a potential `type-of` function
would work in this calculus:

Type-of :=

- (Abs) `λ (v : e1) -> e2` => `λ (v : e1) -> type-of e2`

- (App) `e1 e2` =>
```
let type-e1 = type-of e1.
let type-e2 = type-of e2.

Force type-e1 to be of shape λ (v : e3) -> e4.

Ensure type-e2 = e3.

Return e4 with e1 substituted for v.
```

- (Var) `v` => `Ctx[v]`


There are a couple things to note here:

- Since term and type languages are unified, `type-of` can use the exact same constructs for its output as for its input.

- Thus, there is no need for the type of functions to be its own construct. Indeed, **the type of a function is itself a function**.

- The (App) case is complicated and does a lot of work.
  There is both an equality check *and* substitution happening.

  Furthermore, forcing `type-e1` to be of a specific shape will not work when the type of `e1` is not known i.e. during type inference.

  Note that the substitution of v in e4 with e1 is what enables **dependent typing**; it allows the "result type" of e1 to be dependent on e2.

- The `Ctx` in (Var) represents the set of bound variables and their types. **In a syntactically valid expression, there are no unbound variables**.

The App-case can be simplified very easily, mirroring the way from the untyped `λ v -> e` to the typed `λ (v : e1) -> e2`: Just add a type annotation.

The annotation of applications is a very natural extension for any calculus that already annotates its bindings in abstractions.

Similarly, the `Ctx` in (Var) can be eliminated by demanding every variable occurrence to be annotated with its type. This is possible since every syntactically valid expression contains only bound variables.

As a result, this is our new calculus and its corresponding `type-of` function:

Expression :=
- (Abs) `λ (v : e1) -> e2`
- (App) `e1 e2 : e3`
- (Var) `v : e`

Type-of :=

- (Abs) `λ (v : e1) -> e2` => `λ (v : e1) -> type-of e2`
- (App) `e1 e2 : e3` => `e3`
- (Var) `v : e` => `e`

**The type of every object is determined either trivially, or recursively without doing any substitution or equality check.**

## The trouble with dependence

Since `type-of` doesn't do any checking, a dedicated `check` procedure is needed that determines the validity of an expression.

This `check` procedure recursively traverses a given expression and tests the validity of every instance of (App) it finds.

This (App) validity testing could look like this:

App-is-valid :=

- `e1 e2 : e3` => `(type-of e1) = λ (v : type-of e2) -> e3`

However, this loses the ability of dependent typing; the "result type" of e1 cannot depend on e2.

Another solution would be, like in first version of `type-of`, to force the shape of `type-of e1` to be an abstraction. This however has the already noted downside of not working with type inference.

Thankfully, there is a third solution that provides the best of both worlds: a new construct called **Equality Maps**.

## Equality to the rescue

The idea is to reify the `e2 : e3` part of (App) as its own construct, written `e1 -> e2`.

This results in another new version of the calculus:

Expression :=
- (Abs) &nbsp;&nbsp;&nbsp; `λ (v : e1) -> e2`
- (App) &nbsp;&nbsp;&nbsp; `e1 ! e2 -> e3`
- (Var) &nbsp;&nbsp;&nbsp;&nbsp;&nbsp; `v : e`
- (E-Map) `e1 -> e2`

The syntax of (App) changed to make the role of (E-Map) in it obvious.

With the addition of (E-Map), `type-of` now looks like this:

Type-of :=
- (Abs) `λ (v : e1) -> e2` => `λ (v : e1) -> type-of e2`
- (App) `e1 ! e2 -> e3` => `e3`
- (Var) `v : e` => `e`
- (E-Map) `e1 -> e2` => `e1 -> type-of e2`

The reason why the (E-Map) case looks the way it does will be made clear after `app-is-valid` is revisited.

With (E-Map), `app-is-valid` now looks like this:

App-is-valid :=
- `e1 ! e2 -> e3` => `(type-of e1) = e2 -> e3`

Sadly, this doesn't quite work. While e2 is now reflected at the type level, enabling dependent typing, the notion of equality is not anymore appropriate in this case.

`type-of e1` is supposed to be an abstraction, yet an abstraction is clearly not equal to any equality map.

Thus, a new notion is needed, one that makes clear the exact relationship between (Abs) and (E-Map): *Subtyping*.

## An unlikely contender: subtyping

Using the relation `e1 <: e2` to mean "e1 is a subtype of e2", `app-is-valid` looks like this:

App-is-valid :=
- `e1 ! e2 -> e3` => `(type-of e1) <: e2 -> e3`

As a perhaps surprising result, **subtyping is necessary for dependent typing**.

As a consequence of using subtyping to determine (App) validity, (App) can not only be used to eliminate abstractions, but anything that "can be treated as" an equality map. This obviously includes equality maps themselves, and is the reason why the type of an equality map is an equality map itself.

It is instructive to imagine what it means for an equality map to be used in (App). Imagine if string literals were part of the language, with 'String' as their type, then an (App) such as this could arise:

`("name" -> "Church") ! "name" -> String`

A `check` on that expression results in the following test:

`("name" -> String) <: "name" -> String`

That test decomposes into `"name" = "name"` and `String <: String`, both of which are obviously true, thus determining the whole expression valid.

The key thing to note is that subtype testing equality maps results in tests for equality of their first, or "input", part, thereby **requiring an equality relation for every object of the calculus**.

This property is what gives equality maps their name.

Equality maps mirror several aspects of abstractions. Both reflect themselves in their types, and both can be eliminated by the same rule. Just like equality maps are mappings based on *equality*, abstractions are mappings based on *type*.

So it makes sense to align their naming. Abstractions will now be called **Type Maps** (T-Map), and (App) will be renamed to **Equality Map Elimination** (E-Map Elim), emphasizing that this rule eliminates every object that can be treated as an equality map.

The new calculus:

Expression :=
- (E-Map) &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; `e1 -> e2`
- (T-Map) &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; `λ (v : e1) -> e2`
- (E-Map Elim) `e1 ! e2 -> e3`
- (Var) &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; `v : e`

## He loves me, he loves me not... he loves me *maybe*?

This is a good time to look at the newly introduced <: operator.

Its definition is given by listing all possible cases exhaustively.

Is-subtype (<:) :=

(E-Map) as subtype:
- `e1 -> e2 <: e3 -> e4` => `e1 = e3` & `e2 <: e4`
- `_ <: (T-Map)` => No
- `_ <: (Var)` => ?
- `_ <: (E-Map Elim)` => ?

(T-Map) as subtype:
- `λ (v : e1) -> e2 <: e3 -> e4` => `(type-of e3) <: e1` & `e2[v := e3] <: e4`
- `λ (v : e1) -> e2 <: λ (v2 : e3) -> e4` => `e3 <: e1` & `e2[v := v2] <: e4`
- `_ <: (E-Map Elim)` => ?
- `_ <: (Var)` => ?

(E-Map Elim) as subtype:
- `_ <: (E-Map)` => ?
- `_ <: (T-Map)` => ?
- `e1 ! e2 -> e3 <: e1 ! e2 -> e3` => Yes
- `_ <: (E-Map Elim)` => ?
- `_ <: (Var)` => ?

(Var) as subtype:
- `_ <: (E-Map)` => ?
- `_ <: (T-Map)` => ?
- `_ <: (E-Map Elim)` => ?
- `v <: v` => Yes
- `_ <: (Var)` => ?

Up until now, it was implictly assumed that tests for equality or subtyping relationship were of Boolean nature; either things are equal/a subtype or not.

However, due to term and type language being one and the same, the domain of these tests include cases like (Var) and (E-Map Elim), for which answering Yes or No would be too strong, or rather, too soon.

To illustrate, assume there are two distinct types Int and String and the following statements:

- (1) Int <: String
- (2) Int <: Int
- (3) Int <: x

(1) is obviously False: A value of type Int can *never* be treated like a String.

(2) is obviously True: A value of type Int can *always* be treated like an Int.

(3) is actually neither True or False.

Remember, a variable represents a value that is yet to be determined.

Saying this statement is True would be too permissive: x could very well be 'String' in some context, making saying 'True' a lie.

Likewise, saying it's False is too restrictive, since x could of course be 'Int' in some context, too.

So the actual result of (3) is that it *could* be True, but also *could not*. It is simply not yet known. This same conundrum occurs with equality in place of subtyping and/or (E-Map Elim) in place of (Var).

What is needed is something between Yes and No that adequately captures this uncertainty, like "Maybe".

Every occurrence of '?' in the definiton of '<:' above can be replaced by 'Maybe'.

Thus, **the result of subtyping and equality tests is _ternary_, not Boolean.**

## Static or dynamic, 'tis all the same.

Since the result of `app-is-valid` can now be Maybe, re-determination needs to happen either as soon as any part of such an (App) changes through evaluation, or at last when evaluation reaches such an (App).

In practice, there will be one `check` before any evaluation occurs; this will catch all errors that are independent of run-time data. These are the types of errors statically-typed languages find.

All other *potential* errors will be labeled Maybe and checked during evaluation: these are the kinds of errors dynamically-typed languages find (in addition to the static errors).

That means errors will be detected as soon as possible, without limiting how dynamic a program can be.

The key takeaway here is that dynamic errors are *not* a different kind of construct than static errors.

**Dynamic errors *are* static errors after a certain number of evaluation steps**.

## Symmetry is harmony

Let's look at our calculus again:

Expression :=
- (E-Map) &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; `e1 -> e2`
- (T-Map) &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; `λ (v : e1) -> e2`
- (E-Map Elim) `e1 ! e2 -> e3`
- (Var) &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; `v : e`

There are two different kinds of maps, and an elimination which uses one of those. It is reasonable to ask if there could be something like a (T-Map Elim), too.

It's not hard to imagine what that would look like syntactically:

- (T-Map Elim) `e1 ! λ (v : e2) -> e3`

To understand what this actually could do, it helps to analyze the cases where e1 is an (E-Map) and a (T-Map):

- `(e1 -> e2) ! λ (v : e3) -> e4`

  This looks like the (E-Map Elim) case with (T-Map) as e1, just reversed. And indeed, the procedure here is nearly the same:
  The type of e1 needs to compatible with e3, e1 gets bound as v in e4, and the type of e2 needs to be compatible with that new e4.
  The result is e2.

- `(λ (v : e1) -> e2) ! λ (v : e3) -> e4`

  The situation here is different. There just no way to do away with either (T-Map). This case simply doesn't work.

So, the first case should work assuming all the types line up, and the second one should never work.

There's a problem with that, however, most readily apparent in the second case.

It's supposed to never work, which implies that `check` has to always answer with No. Yet, suppose this instance:

`(λ (_ : Int) -> 42) ! λ (_ : Int) -> Int`

That will result in the following check:

`(λ (_ : Int) -> Int) <: λ (_ : Int) -> Int`

which is obviously true, the opposite of what is actually wanted.

What is needed is a type map that is practically the same as the existing one except different, so that they are not inherently compatible.

They are pretty much infinite ways to syntactically mark and name this new kind of type map; here it's called **Negative Type Map** (Neg T-Map), written `λ (v : e1) ~> e2`.

Crucially, we define `(λ (v : e1) -> e2) <: λ (v2 : e3) ~> e4` to be always false, regardless of e1, e2, e3 and e4.

The existing (T-Map) gets renamed appropriately, resulting in yet another new calculus:

Expression :=
- (E-Map) &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; `e1 -> e2`
- (Pos T-Map) &nbsp; `λ (v : e1) -> e2`
- (Neg T-Map) &nbsp; `λ (v : e1) ~> e2`
- (E-Map Elim) `e1 ! e2 -> e3`
- (T-Map Elim) `e1 ! λ (v : e2) ~> e3`
- (Var) &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; `v : e`

## All or ~~nothing~~ at least one

To gain some intuition for the meaning of negative type maps as types, it is helpful to imagine a negative type map to be a shorthand notation for the *disjunction* of all equality maps satisfying the type map's relationship between 'input' and 'output'.

As a (very simple) example, one could view `λ (_ : Int) ~> Int` as a shorthand for `(0 -> Int) or (1 -> Int) or (2 -> Int) ...`. An instance of such a type would be any of `0 -> 0`, `0 -> 1`, `11 -> 42` etc.

In other words, a negative type map represents **at least one** equality map satisfying the type map's relationship.

Analogously, a positive type map could be viewed as a shorthand for the *conjunction* of all equality maps satisfying its relationship; it represents *all* of those maps.

This dichotomy of 'all' and 'at least one' is reminiscent of quantification in logic with its 'for all' and 'there exists' quantifiers. And indeed, **positive type maps correspond directly to universal quantification**, and **negative type maps corrrespond directly to existential quantification**.

What remains now is defining what a negative type map means as a *value*, or in other words, figuring out what type a negative type map should have.

(T-Map) split into two polarized versions as a result of its interpretation as a type. However, on the value level there is no reason for any bifurcation. **Both positive and negative type maps can be used interchangeably as values**, and thus have the same type.
That type is a positive type map.


## Polarity strikes again

In our current system, it's possible to construct a chain of subtype checks like this:

`(λ (_ : Int) -> Int) <: (1 -> Int) <: (λ (_ : Int) ~> Int)`

Each individual subtype check makes sense, but taken together mean that a situation can arise where one can take a positive type map and treat it like a negative one, something we explicitly try to avoid.

Mirroring the approach to the (T-Map) problem above, we split (E-Map) into positive and negative versions.

And just like (T-Map Elim) features a negative type map, (E-Map Elim) changes to feature the negative equality map.

This combined with making `(Pos T-Map) <: (Pos E-Map)` always false ensures that the above conundrum cannot appear.

## So far, so good

Let's take a look at the calculus so far:

Expression :=
- (Pos E-Map) `e1 -> e2`
- (Neg E-Map) `e1 ~> e2`
- (Pos T-Map) &nbsp; `λ (v : e1) -> e2`
- (Neg T-Map) &nbsp; `λ (v : e1) ~> e2`
- (E-Map Elim) `e1 ! e2 ~> e3`
- (T-Map Elim) `e1 ! λ (v : e2) ~> e3`
- (Var) &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; `v : e`


Type-of :=
- `e1 -> e2` => `e1 -> type-of e2`
- `e1 ~> e2` => `e1 -> type-of e2`
- `λ (v : e1) -> e2` => `λ (v : e1) -> type-of e2`
- `λ (v : e1) ~> e2` => `λ (v : e1) -> type-of e2`
- `e1 ! e2 ~> e3` => `e3`
- `e1 ! λ (v : e2) ~> e3` => `e3`
- `v : e` => `e`


Is-subtype (<:) :=

(Pos E-Map) as subtype:
- `e1 -> e2 <: e3 -> e4` => `e1 = e3` & `e2 <: e4`
- `e1 -> e2 <: e3 ~> e4` => `e1 = e3` & `e2 <: e4`
- `_ <: (Pos T-Map)` => No
- `e1 -> e2 <: λ (v : e3) ~> e4` => `(type-of e1) <: e3` & `e2 <: e4[v := e1]`
- `_ <: (E-Map Elim)` => Maybe
- `_ <: (T-Map Elim)` => Maybe
- `_ <: (Var)` => Maybe

(Neg E-Map) as subtype:
- `_ <: (Pos E-Map)` => No
- `e1 ~> e2 <: e3 ~> e4` => `e1 = e3` & `e2 <: e4`
- `_ <: (Pos T-Map)` => No
- `_ <: (Neg T-Map)` => No
- `_ <: (E-Map Elim)` => Maybe
- `_ <: (T-Map Elim)` => Maybe
- `_ <: (Var)` => Maybe

(Pos T-Map) as subtype:
- `_ <: (Pos E-Map)` => No
- `λ (v : e1) -> e2 <: e3 ~> e4` => `(type-of e3) <: e1` & `e2[v := e3] <: e4`
- `λ (v : e1) -> e2 <: λ (v2 : e3) -> e4` => `e3 <: e1` & `e2[v := v2] <: e4`
- `_ <: (Neg T-Map)` => No
- `_ <: (E-Map Elim)` => Maybe
- `_ <: (T-Map Elim)` => Maybe
- `_ <: (Var)` => Maybe

(Neg T-Map) as subtype:
- `_ <: (Pos E-Map)` => No
- `_ <: (Neg E-Map)` => No
- `_ <: (Pos T-Map)` => No
- `λ (v : e1) ~> e2 <: λ (v2 : e3) ~> e4` => `e1 <: e3` & `e2[v := v2] <: e4`
- `_ <: (E-Map Elim)` => Maybe
- `_ <: (T-Map Elim)` => Maybe
- `_ <: (Var)` => Maybe

(E-Map Elim) as subtype:
- `_ <: (Pos E-Map)` => Maybe
- `_ <: (Neg E-Map)` => Maybe
- `_ <: (Pos T-Map)` => Maybe
- `_ <: (Neg T-Map)` => Maybe
- `e1 ! e2 ~> e3 <: e1 ! e2 ~> e3` => Yes
- `_ <: (E-Map Elim)` => Maybe
- `_ <: (T-Map Elim)` => Maybe
- `_ <: (Var)` => Maybe

(T-Map Elim) as subtype:
- `_ <: (Pos E-Map)` => Maybe
- `_ <: (Neg E-Map)` => Maybe
- `_ <: (Pos T-Map)` => Maybe
- `_ <: (Neg T-Map)` => Maybe
- `_ <: (E-Map Elim)` => Maybe
- `e1 ! λ (v : e2) ~> e3 <: e1 ! λ (v : e2) ~> e3` => Yes
- `_ <: (T-Map Elim)` => Maybe
- `_ <: (Var)` => Maybe

(Var) as subtype:
- `_ <: (Pos E-Map)` => Maybe
- `_ <: (Neg E-Map)` => Maybe
- `_ <: (Pos T-Map)` => Maybe
- `_ <: (Neg T-Map)` => Maybe
- `_ <: (E-Map Elim)` => Maybe
- `_ <: (T-Map Elim)` => Maybe
- `v <: v` => Yes
- `_ <: (Var)` => Maybe

This is already a pretty neat calculus. We can construct individual mappings (E-Maps) and families of mappings based on the relationship of the output to the input (T-Map).

As we've seen with the explanation of positive and negative type maps however, there is a need to be able to represent the conjunction and disjunction of objects in our calculus.

## Better together (except during a pandemic)

Representing con- and disjunction is pretty easy: We use a binary infix '&' for conjunctions, and a binary infix '|' for disjunctions.

Defining their role in subtyping helps to understand their behavior as a type:

Conjunction:
  - `A & B <: X` => `A <: X` | `B <: X`

    Intuition: I want X and you've given me A and B. I'm satisfied if at least one of A or B is compatible with X.

  - `X <: A & B` => `X <: A` & `X <: B`

    Intuition: I want both A and B and you've given me X. I'm satisfied if X is compatible with both A and B.

Disjunction:
  - `A | B <: X` => `A <: X` & `B <: X`

    Intuition: I want X and you've given me A or B. I'm satisfied if both of A or B is compatible with X.

  - `X <: A | B` => `X <: A` | `X <: B`

    Intuition: I want either A or B and you've given me X. I'm satisfied if X is compatible with at least one of A or B.

Just like equality maps and type maps, conjunction and disjunction are essentially the same construct, differing only in their behavior as a type and are distinguised by only 1 bit of information.

Going forward, conjunction will be named `positive junction` (Pos Jun) and disjunction `negative junction` (Neg Jun).

Their interpretation as a value follows the pattern too, in that both can be used interchangeably on the value level. Therefore, they both have the same type; positive junction:

`type-of (e1 & e2)` => `(type-of e1) & (type-of e2)`

`type-of (e1 | e2)` => `(type-of e1) & (type-of e2)`

These type-of rules makes it easy to see how to construct an object of type (Pos Jun); just form either positive or negative junctions.

As an example, assume that string literals and integers are part of the calculus:

`(("name" -> "Simon") & ("age" -> 62)) ! "age" ~> Int`

This would lead to the check:

`("name" -> String) & ("age" -> Int) <: "age" ~> Int`

decomposing into:

`"name" -> String <: "age" ~> Int`

which fails, and

`"age" -> Int <: "age" ~> Int`

which succeeds, making the whole check succeed.

On the operational side, the process is very similar: First try the first part of the conjunction, then if it failed, the second.

The result of this particular expression is, unsuprisingly, `62`.

This example should be reminiscent of an existing construct present in most programming languages: records. Only in this calculus, **records are not a primitive construct**, but instead the emergent behavior of completely orthogonal constructs.

This is an example of a wider pattern with DCC: Exisiting constructs present in other programming languages get reformulated in more fundamental terms, resulting in a simpler, easier to understand yet equally if not more powerful language.

Also, note how eliminating a conjunction is the same as indirect branching.

## Do. Or do not. There is no try. (He's lying, there totally is.)

How do negative junctions come about, however? While their existence is already necessitated by positive junctions alone, there is also a very interesting new construct whose type is best characterized by negative junctions.

Remember that every elimination in DCC at any given time is either known to succeed or fail, or be indeterminate. It would be neat if there was a construct that could act upon that information, behaving one way if the elimination is ultimately successful, and behaving another if it is not.

That construct is called Alternative (Alt), written as `else`, and acts as a binary infix operator, like the junctions above.

In contrast to the junctions however, 'else' only has meaning on the value level. As a type it is more similar to eliminations and variables: being present in expressions evaluation hasn't reached yet.

Its behavior is relatively straightforward:

Evaluating `e1 else e2`:
- Evaluate e1. If it succeeds, yield e1.
- If it fails, evaluate e2 and yield the result of that.
- If the result is indeterminate, yield `e1 else e2`.

In this way, 'else' implements direct branching without needing an if-like construct. Instead of formulating Boolean expressions and acting on that, DCC uses the ternary status inherent in all of its objects.

'Else' is an expression that is either one or the other of its constituents, and its type reflects that:

`type-of (e1 else e2)` => `(type-of e1) | (type-of e2)`

{ Further content coming soon. }
