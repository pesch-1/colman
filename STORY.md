**English** | <!-- optional: [Deutsch](STORY.de.md) -->

# The story of colman

*How a school library manager came into being — and why large parts of it
were written together with an AI assistant.*

---

## Contents

1. [Where it all started](#where-it-all-started)
2. [The revival](#the-revival)
   - [Technology choices](#technology-choices)
3. [Why AI?](#why-ai)
4. [How I worked with AI](#how-i-worked-with-ai)
5. [Vibe coding a Flutter client](#vibe-coding-a-flutter-client)
6. [Conclusion](#conclusion)

---

## Where it all started

When I started developing colman in 2015, AI played no part in it. The idea
was to write a small program to help my wife manage the textbooks at her
school. The focus was on class sets: many copies of the same book are bought,
and a class might, for example, borrow 20 of 60 copies and return 15 of them a
year later. You want to record each of these transactions in a single step
instead of inventorying every copy individually. Existing software did not
support this use case well. At the same time, I wanted to take a look at Qt's
support for SQL databases and brush up on database skills that had been lying
dormant for two decades. That is how the first version of colman came about.
Apparently I had built it to be quite robust: it stayed in use unchanged for a
long time, while the details of its implementation gradually faded from my
memory.


## The revival

When my wife moved to a different school, a new requirement came up: the
software should also work for a student library, where students borrow and
return books themselves. colman did not cover these use cases well, and since
I do not have much spare time for this project, my first advice was to use
existing software. It soon became clear, however, that this would also mean
plenty of work for me — from helping with the installation, through
configuration and adapting it to special requests, to the frustration when
certain workflows simply could not be implemented the way they were wanted.

At the same time, I wanted to try out the new AI tools for software
development and form my own picture of their opportunities and risks. There
are so many speculative opinions out there; I wanted first-hand experience.
What already works and what doesn't? How do you use these tools best? Which
ones work well for me? And how expensive does it get? Continuing the
development of colman with the help of AI was the obvious choice.


### Technology choices

Since it was foreseeable that the standalone application would eventually
grow into a client-server solution — one school server and many mobile
clients — I refactored the software so that the same code base can be used
both for a local application and for a server with a REST API.

For the desktop client, it was clear that I wanted to stay with Qt, to remain
as flexible as possible and to reuse existing code. I also experimented with a
browser client. That is certainly feasible, but the additional tools it
requires (authentik, nginx and the like) would have added unnecessary
complexity. Installation would have been anything but user-friendly, the clean
separation between the GUI in the client and the data on the server would have
suffered, and debugging would have become considerably harder whenever
something went wrong — the exact opposite of "keep it simple". On the other
hand, the benefits of a browser client are not significant enough to justify
all that. Should a compelling reason come up after all, a browser client can
still be added at any time.

No mobile clients are in real-world use yet. Nevertheless, I have started
implementing one, and the choice fell on Flutter. I wanted to try out two
things: how well Flutter is suited to applications that run on both Android
and iOS, and whether something like this can be built by vibe coding. Since I
do not own any Apple devices, the iOS app has never been built or tested — if
you would like to help with that, you are very welcome! The Android app is
usable, but not yet complete and not as thoroughly tested as the desktop
application. Feedback and bug reports are much appreciated here as well
(although, of course, I hope there are as few bugs as possible).

For the server, I decided to stick with a simple SQLite database for now. From
the user's point of view, this means minimal installation effort, and it is
perfectly adequate for small to medium-sized collections. The software is
structured, however, so that switching to a more powerful database is possible
at any time.


## Why AI?

Using AI assistance suggested itself for two reasons. First, I lacked the time
to implement all the extensions to colman I had in mind. But above all, I
wanted to gain experience with AI. Although I have been training neural
networks for image processing and object detection for many years, I had no
experience with LLMs and coding agents. So I tried out various levels of AI
involvement.

I quickly settled on a small subscription plan for Claude, which fits my usage
quite well. Since I only work on colman in my spare time, I rarely hit the
five-hour usage limit — which would be a real nuisance for serious
professional work. I also tried cheaper models billed per token, but in total
they were not any cheaper, and their quality could not quite keep up. (That
says nothing about where they will be in the future, though; the game is
changing rapidly.)

Personally, I use Claude for code completion in the editor and as a chat in
the terminal. For a while I also used the chat inside the editor, where Claude
can even control the entire editor. I found, however, that I am more efficient
with the split setup, and the Claude chat does not get in my way when I don't
need it. That is a matter of taste, of course, and certainly depends on one's
habits.


## How I worked with AI

I started by having Claude explain my own code to me. The documentation I had
written a few years earlier was not bad at all, but Claude was far more
efficient at reminding me what I had actually done back then. Encouraged by
this, I had Claude complete the source code documentation, and I could not
find any serious mistakes. I also had Claude write a large number of unit
tests. A modern AI is really well suited to tasks like these and can
contribute a lot to improving code quality.

Then it was time for the next step. I had Claude assess design changes and
received good feedback. It felt a bit like a productive discussion between
developers, and I was pleasantly surprised. My requests became more
demanding. I let Claude implement the necessary SQL functions on its own —
and test them, of course. This, too, worked very well.

By then, I was mainly using Claude Opus. I had Opus rework the GUI and replace
the Qt `.ui` files with C++ code, which was no problem for it either. I have
to admit that Opus has more patience than I do for the small details of a GUI
("make all these buttons the same height", "compute a sensible minimum width
here", "adjust the colors there", ...). Meanwhile, Bing Image Creator produced
the lovely owl icons. I have no talent whatsoever for drawing anything like
that, and I am thrilled by what is possible. This was definitely a task I
could never have accomplished without AI, no matter how much time I had.

Code reviews followed (my tip: repeat them in independent sessions until
nothing new turns up), along with redesigns of dialogs and workflows and the
implementation of a REST API. Again and again, Opus spared me tedious research.
What I would first have had to read up on laboriously, Opus already knows from
its training, so all that is left for me is to verify and test. With each new
Opus version, and as I gained experience in which tasks to hand to the AI and
how to split and phrase them, its wrong decisions became ever fewer and less
significant. What Opus 5.5 achieves today is fascinating.

It reminds me a little of evolution. I roughly understand how it works, yet I
still find it hard to grasp how a wolf could turn into a Chihuahua in what is,
in evolutionary terms, the blink of an eye. It is similar with what we call
AI. I know how it works. But that it can imitate us so well and deliver such
good results is more than astonishing — a few years ago I would have
considered it impossible. Perhaps it says more about ourselves than about AI
that, with a lot of computing power and a few clever ideas, but without any
mathematics beyond an undergraduate course, we turn out to be so easy to
imitate. We keep learning more and more about what is **not** exclusively
human. The complexity emerges in high-dimensional spaces, and that is where my
intuition fails me. This is why I am cautious about predicting what AI will
eventually be able to do and what it never will — some people go quite far
out on a limb there. In my opinion, speculating about whether AI could ever
achieve true intelligence is pointless as long as we don't even know exactly
how our own intelligence works.


## Vibe coding a Flutter client

But back to the topic. I know nothing about Flutter, so implementing a Flutter
client was a wonderful playground for vibe coding. I am amazed at how well it
works. Some of the code strikes me as overly convoluted or inelegant, but the
AI needed only a fraction of the time I would have had to invest myself. And
perhaps I have to swallow the bitter pill that clean design and lean code
become somewhat less important when the next version of a program is
implemented by an AI rather than by a new colleague. In some places I had
newer Opus versions improve the code written by older ones — the progress of
the AI models is clearly noticeable.


## Conclusion

I would not (yet) let an AI develop the control system of a nuclear power
plant or an aircraft autopilot. I would, however, have such software
additionally reviewed and tested by an AI, and I would let an AI improve and
extend its documentation. For relatively simple, non-critical applications
like this school library manager, I am indeed willing to delegate many tasks
to an AI. I have worked with enough developers who wrote considerably worse
code.

In any case, with the time available to me, I would never have reached
colman's current scope without AI — and colman would not have such beautiful
icons either. So I have no qualms about including AI-generated code in
colman. I am happy to share some of the credit with the AI, and I hope you
enjoy colman all the same, even though not every line of it was written by a
human hand.

Christoph Pesch, November 2026

<!-- EOF -->
