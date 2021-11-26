# CrowdStrike Submission - Dante Criscio

## My Submission Folder
The final version of this assignment, and the version which I am submitting for review, can be found in the folder Final_Submission.

Within that folder, the file solution.c has the capability to read all URLs from the file URLs.txt (which simply separates URLs onto different lines) and then connect to these URLs using sockets. To parse the URLs into their addresses and port number I made use of a library known as http-parser, which is well respected and known for simply parsing HTTP messages written in C. It includes a function known as http_parser_parse_url(), which parses URLs into their various components. This is what I used to extend my knowledge of socket programming to URLs. For more information about this library look at http://github.com/nodejs/http-parser. 

Also, within that folder we have a file known as server.c. This forks multiple times to create six different servers. They are all on the localhost but are on different port numbers. These each of these six servers open one csv file, found in the data folder, and send all that information to the client after a connection is made. This server file exists so that my code can be easily tested. To test the code, first compile server.c and solution.c (I have included a makefile which does this compiling on Linux). Next run the server file and then run the solution file. The solution file should connect to all servers and then return the mean, median, someone at the median, and all URLs it had difficulty with.

## The Other Two Folders
The folder Local_Solution shows the first version I made, where the "URLs" are just paths to csv files on the local machine. I have included it because I showcases the ideas behind my solution very clearly without getting bogged down by the intricacies of socket programming. This solution was later reworked to become the final version which I am submitting.

The folder Local_Timing is very much like my local submission, except it includes timing from the very beginning of main() to the end of main(). Additionally, it only accesses file1.csv, file2.csv, file3.csv, and file4.csv but it repeats this process 2500 times. This is meant to create the illusion of calling 10000 URLs. On my machine it takes a little less than 4 seconds (on average) to do this. 

## Mean, Median, and Someone at the Median
When thinking of how to return the median values, my first thought was to put all the ages into an array, sort the array with quicksort, and then return the middle index. This has a running time of O(nlog(n)). I also was planning to use a hash map to hash each age to someone's name at the age. But I then realized that, in order to be a valid age, the ages need to be integers from 0 to 127 (which is older than the oldest person who ever lived). So, I can use a non-comparison sorting algorithm known counting sort, where I just make a global integer array of 128 elements, initialized to 0s. Then, whenever we read a valid age, we increment the value at that index. Once this global array exists it is very easy to find the mean and median, without even needing to actually sort anything fully.

The fact that all ages are integers from 0 to 127 also makes it easy to return the name of someone at the median. We simply create another global array (this one stores full names at each index) where each index is initialized to null. Whenever we encounter a valid age, we then check to see if the name at that index is null, and if so, we put that person's name there. This creates a first-come-first-serve basis for filling up the array. It functions like a hash map, except there is no need to hash the ages since they are already positive integers.

Clearly, my solution depends heavily upon the assumption that all valid ages are integers from 0 to 127. Making use of this assumption, we get very fast code, as the entire solution is O(n) where n is the number of ages. Additionally, the code is fairly light on memory usage, as it only requires two globals, which are arrays of 512 bytes (128 x 4).

## Sanity Checking
To sanity check the URLs I decided to use the following approach.
1. First try to connect to the URL. If a connection is not made print out "Problem connecting to " and the current URL.
2. Read the first 18 bytes from this socket, and ensure that they are "fname, lname, age\n". This is a good way to make sure that our connection is sending over the type of data that we desire. 
3. For each row, check that the row has 3 elements, where the last (the age) is an integer. If some row does not meet this format, we simply skip this row. But we do set a flag which indicates that we had trouble unpacking this file. Once the file is done being read, we print "Problem unpacking " and the current URL. This is also printed if condition 2 is not met.

I am happy with this approach to sanity checking because it very quickly makes sure that we are receiving a csv file as desired, but it also has the flexibility to still extract data from the file even if it is not formatted perfectly.

## Q1 - Assumptions
As previously stated, one big assumption that I made in this approach was the fact that the ages are integers from 0 to 127. This is a very good assumption as we are talking about humans in this project, but if the assumption did not hold (say if we had data about trees or planets) the code would have to be reworked. To handle this scenario, I would use the previous approach where we sort all of the ages using quicksort() and hash all of the ages to a name at that age.

Another very large assumption that I made was about the protocol of the servers that are sending over the data in CSV format. I assumed the simplest cast, which is that the servers simply send all of the data as fast as they can once a connection has been established (this is what my servers do). If some other protocol was needed, like if the client had to introduce itself first in some way, then I would have to redesign my code to account for that. Additionally, if there were several different protocols that could be required based on the URL's protocol, then I would likely have to make use of an external library (since there are simply so many protocols in existence).

Any other assumptions that I made pertain to the size of things, for example I assumed that the maximum URL size is 1028 bytes. All these assumptions can be found as macros at the very beginning of my solution, so they can be modified very easily. For example, if you wanted to account for URLs up to 2056 bytes, you could simply change the macro MAX_URL_SIZE from 1028 to 2056.

## Q2 - Many Files, over 10M Records
The first approach that I would take to speed up my program so that it could handle more data would be to use multi-threading, where each thread would connect to a different subset of URLs from the URL file. Assuming that we have a machine of 4 cores, this would ideally speed the process up by 4x.

However, if each file is over 10M records this would still create a problem where the files themselves are simply too big and each thread would still spend a very long time on a single file. To approach this, I would have to make use of statistical sampling, where we sample a subset of the data from the CSV files, and we assume that the mean and median of these samples reflects the true median and mean. To implement this, I would include a command line argument which is the maximum number of data points that we want to sample from each file (say 10k for example). Each thread would then move to a random spot in the csv file, move backwards until it found a '\n', then read the next row. It would repeat this process based on the number of samples that are desired.

## Q3 - 10k URLs
Assuming that file1.csv, file2.csv, file3.csv, and file4.csv are a good indication of the types of files that each URL would be sending over, I would argue that my code can already handle 10k URLs. This is the reason why I included the folder Local_Timing. However, if speedup is still desired (say we want a running time under 1s). Multithreading would be perfect for this.
