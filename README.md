# Longest Living Pixels of r/place

## Compliation and running

This C++17 program uses the sorted pixel history to output the longest living
pixels.
It's just one file and it doesn't use any non-standard libraries, hopefully
whatever compiler you want to use should work.
I personally used `g++ longest_living.cpp -o longest_living.out`.

The 2022 pixel history is at
<https://placedata.reddit.com/data/canvas-history/2022_place_canvas_history.csv.gzip>,
but it isn't sorted by placement time.
Since the timestamp is the first field, and the formatting works nicely, a
string comparison is sufficient, and directly using
[UNIX sort](https://man7.org/linux/man-pages/man1/sort.1.html)
sorts the data by placement time.
My sorted data has SHA-256 `a083da7cbb656ef68204b99ff3258b1f94ca6ca63053b7d401df424e21e704c6`.

Then, running
`./longest_living.out <path_to_sorted_history> [number of pixels = 1000000]`
will output by default the top 1000000 longest living pixels, or however many
are specified on the command line.
The output is csv with fields
`duration(ms),placement time(ms),uid(hex),color(hex),x,y`.
The uid is not in base64 like the pixel history since I didn't implement a
base64 encode.
The rank of a specific pixel is the number of pixels outputted minus the line
number of the pixel plus one.

## Results

My uid was
`YyEHvABKGtz58DCwPt8Zy6WCi+vl9RaoiwEAusvS7wFERszRfnOOAhXncaWauNMrxXpFu/x0tE23FtizENPDfQ==`
in base64, or
`632107bc004a1adcf9f030b03edf19cba5828bebe5f516a88b0100bacbd2ef014446ccd17e738e0215e771a59ab8d32bc57a45bbfc74b44db716d8b310d3c37d`
in hex.
Some of my pixels were in the top 1000000:

    Rank 951633 : 95657551,1649020320410,<uid>,000000,1452,1284
    Rank 940114 : 96001246,1649019684545,<uid>,00756f,1464,1273
    Rank 926412 : 96400059,1649018654211,<uid>,00756f,1469,1281
    Rank 787299 : 101100619,1649012125064,<uid>,000000,920,88
    Rank 551266 : 117116124,1648996164141,<uid>,00756f,1647,303
    Rank 468463 : 125377548,1648929841627,<uid>,ffd635,563,22
    Rank 209365 : 169312874,1648943706680,<uid>,000000,1729,785
    Rank 199758 : 171580984,1648942464916,<uid>,000000,1721,787
    Rank 199453 : 171651356,1648943404151,<uid>,000000,1725,784

Since there were roughly 160353105 pixel placements total (`wc -l <history>`), I
had 9 pixels in the top .6% of all pixels placed.
I think this means I need to spend more time outside.
