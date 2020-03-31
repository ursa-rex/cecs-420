James Bryant    ID#5194642
Ted South       ID#5163470

No, we did not fully implement the project as described.

1. The description specified that we should've used threads for
the workers and one for the sender. At the time of implementation,
we found it easier to make threads for the workers and a regular
function for the sender.

2. The mapper does not successfully send every map item in the
buffer to the message queue.

3. The reducer does not print the map items to a file

We chose to use 256B for the messages. While that was the max word
size, I thought it was enough space for the rest of the parts of the
map items since there were no words in the files that was exceed
that size.