#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PRICECOL 4

// Utility to convert QuantQuote "SPY_SECOND_TRADE.csv" file to a stream of seconds data
// Change filenames in the source. Doesn't need any user input. Takes under a second.
// There are some spikes are in the data, but don't bother filtering since they are few.

char infilename[200] = "D:/EEG/SPY_SECOND_TRADE.csv";
char outfilename[200] = "stock.txt";

int main()
{
	FILE *infile, *outfile;

	infile = fopen(infilename, "r");
	if (infile == NULL)
	{
		printf("Bad input filename\n");
		return 1;
	}

	outfile = fopen(outfilename, "w");
	if (outfile == NULL)
	{
		fclose(infile);
		printf("Bad output filename\n");
		return 2;
	}

// Process a line at a time. The line includes the EOL marker.

	char * line = NULL;
	size_t len = 0;
	size_t read;
	size_t linenum = 0;
    size_t seconds = 0;
    double oldprice = 0;
    fprintf(outfile, "100 StockSeconds\n");

	while ((read = getline(&line, &len, infile)) != -1) {
//		printf("Retrieved line %d of length %zu:\n", linenum+1, read);
//		printf("%s", line);
		if (linenum) {                  // skip the header line
            char *token;
            token = strtok (line, ","); // parse out 1st column
            long int time = (strtol(token,NULL,10)) / 1000;
            for (int i=0; i<PRICECOL; i++) {
                token = strtok (NULL, ",");
            }
            double price = (double)strtol(token,NULL,10) * 0.01;
// At this point, time is in seconds and price is in cents
//          printf("%ld, %.2f\n", time, price);
            if (!seconds) {             // first point
                oldprice = price;
                fprintf(outfile, "%.2f\n", oldprice);
                seconds = time;
            } else {                    // ramping from last price
                int span = time - seconds;
                double slope = (double)(price - oldprice) / span;
//              printf("span=%d, slope=%f, from %.2f\n", span, slope, oldprice);
                for (int i=0; i<span; i++) {
                    oldprice += slope;
                    fprintf(outfile, "%.2f\n", oldprice);
                }
                seconds = time;
            }
		}
		linenum++;
	}

	fclose(infile);
	fclose(outfile);
	return 0;
}
