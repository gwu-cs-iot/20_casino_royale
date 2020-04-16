#include <iostream>
#include <string.h>
#include <time.h>
#include <thread>
//#include "mingw.thread.h" // needed for multithreading with MinGW on Windows

using namespace std;

// for HandRanks.dat
int HR[32487834];

// for cardNametoInt (prints out card value from given int, will keep in for QR testing)
const char cardNames[53][3] = { "??", // unknown card
	"2c", "2d", "2h", "2s", "3c", "3d", "3h", "3s",
	"4c", "4d", "4h", "4s", "5c", "5d", "5h", "5s",
	"6c", "6d", "6h", "6s", "7c", "7d", "7h", "7s",
	"8c", "8d", "8h", "8s", "9c", "9d", "9h", "9s",
	"tc", "td", "th", "ts", "jc", "jd", "jh", "js",
	"qc", "qd", "qh", "qs", "kc", "kd", "kh", "ks",
	"ac", "ad", "ah", "as" };

void printHand(int* pCards, int* cCards);
void printHand(int* hand);
int cardNametoInt(char* cardName);
int* combinePlyCommunity (int* pCards, int* cCards);
int lookupHand(int* pHand);
int lookupHand(int* pCards, int* cCards);
bool possibleCard(int card, int *pCards, int* cCards);
float calcOdds7(int score, int* pCards, int* cCards);
float calcOdds6(int* pCards, int* cCards);
float calcOdds5(int* pCards, int* cCards);
float avgScore6(int* pCards, int* cCards);
float avgScore5(int* pCards, int* cCards);
float calcOdds(int* pCards, int* cCards);
float calcAvgScore(int* pCards, int* cCards);
int* generateRandomHand(int numCards);
void bulkTest7(int numTrials, float winOdds);
void bulkTest6(int numTrials, float winOdds);
void bulkTest5(int numTrials, float winOdds);


// For bulktest
static int riverExpectedWin = 0;
static int riverWin = 0;
static int riverLose = 0; 

static int turnExpectedWin = 0;
static int turnWin = 0;
static int turnLose = 0;

static int flopExpectedWin = 0;
static int flopWin = 0;
static int flopLose = 0;

int main(int argc, char* argv[]) {
	// CHANGE TO THE PATH OF HandRanks.dat, will vary by system/OS
	// Windows directory: "C:\\Users\\nikor\\Downloads\\20_casino_royale-oddsAnalysis\\TwoPlusTwoHandEvaluator\\HandRanks.dat"
	//
	char HandRanksLoc[] = "/home/nikorev/20_casino_royale/TwoPlusTwoHandEvaluator/HandRanks.dat";


	// Open HandRanks.dat and load it into memory
	memset(HR, 0, sizeof(HR));
	FILE * fin = fopen(HandRanksLoc, "rb");
	if(!fin) {
		printf("Failed to open HandRanks.dat\n");
		printf("Generate using Makefile in TwoPlusTwoHandEvaluator\n");
		return -1; // terminate program, we need table
	}
	size_t bytesread = fread(HR, sizeof(HR), 1, fin);
	fclose(fin);
	
	
	// NOW WE CAN BEGIN
	
	// -qt flag for quick test (manual odds input)
	if( (argc > 1) && (strcmp(argv[1], "-qt") == 0) ) {
		if(argc < 7) {
			printf("Missing arguments\n");
			return -1;
		}

		int pCards[2] = {cardNametoInt(argv[2]), cardNametoInt(argv[3])};

		int* cCards = (int*)malloc(sizeof(int) * 5);
		memset(cCards, 0, sizeof(cCards));


		for(int i = 0; i <= (argc - 5); i++) {
			cCards[i] = cardNametoInt(argv[i + 4]);
		}
		for(int i = (argc - 5) + 1; i <= 5; i++) {
			cCards[i] = 0;
		}

		int* hand = combinePlyCommunity(pCards, cCards);

		int currentScore = lookupHand(pCards, cCards);
		float avgPossibleScore = calcAvgScore(pCards, cCards);
		float odds = calcOdds(pCards, cCards);

		printHand(pCards, cCards);

		printf("currentScore: %d\navgPossibleScore: %f\nOdds:%f\n", currentScore, avgPossibleScore, odds);
		free(cCards);
		free(hand);
		return 0;
	}

	// -bt gets accuracy of odds by making two opponents play against each other in a bernoulli experiment
	// If greater than 50% odds, we should expect a win
	// This test does the following: generates a random hand
	// 	gets win percentage on flop, turn, and river
	//		if > 50%, we have an expected win
	//	reveal all cards (turn and river if not doing all 7 off the bat)
	//	compare scores with a random opponent hand
	//		if we win
	//			flop/turn/river win++
	//		else
	//			lose++
	// ./cardOdds -bt <numTrails> <numOpponents>
	// will take a long time, run with >1000 trials which takes around a minute on a 2011 macbook pro
	if( (argc > 1) && (strcmp(argv[1], "-bt") == 0) ) {

		if(argc != 3) {
			printf("Invalid arguments");
			return -1;
		}
		
		printf("Up to %d concurrent threads supported\n", std::thread::hardware_concurrency());

		// bulktest-results.txt
		FILE * btout = fopen("bulktest-results.txt", "w");
		int numTrials = atoi(argv[2]);
		float winOdds = .5; // > 50% means we should win (2 opponents)
		
		time_t startTime = time(0);
		srand(startTime); // to generate a new seed, for proper random cards

		fprintf(btout, "Bulk Test with %d Trials (>50%% odds = expected win)\nStarted Execution at: %s\n\n", numTrials, asctime(localtime(&startTime)));

		printf("Beginning Bulk Test on %d Trials (for 5/6/7 card hands)\n", numTrials);	
		
		// 7 CARD
		//bulkTest7(btout, numTrials, winOdds);
		thread bt7Thread1(bulkTest7, numTrials/4, winOdds);
		thread bt7Thread2(bulkTest7, numTrials/4, winOdds);
		thread bt7Thread3(bulkTest7, numTrials/4, winOdds);
		thread bt7Thread4(bulkTest7, numTrials/4, winOdds);
		
		bt7Thread1.join();
		bt7Thread2.join();
		bt7Thread3.join();
		bt7Thread4.join();
		
		// 6 card
		//bulkTest6(btout, numTrials, winOdds);
		thread bt6Thread1(bulkTest6, numTrials/4, winOdds);
		thread bt6Thread2(bulkTest6, numTrials/4, winOdds);
		thread bt6Thread3(bulkTest6, numTrials/4, winOdds);
		thread bt6Thread4(bulkTest6, numTrials/4, winOdds);
		
		bt6Thread1.join();
		bt6Thread2.join();
		bt6Thread3.join();
		bt6Thread4.join();

		// 5 card
		//bulkTest5(btout, numTrials, winOdds);
		thread bt5Thread1(bulkTest5, numTrials/4, winOdds);
		thread bt5Thread2(bulkTest5, numTrials/4, winOdds);
		thread bt5Thread3(bulkTest5, numTrials/4, winOdds);
		thread bt5Thread4(bulkTest5, numTrials/4, winOdds);
		
		bt5Thread1.join();
		bt5Thread2.join();
		bt5Thread3.join();
		bt5Thread4.join();

		double elapsedSeconds = difftime(time(NULL), startTime);
		
		fprintf(btout, "7 Card Results (%d hands)\nExpected Win %%: %f (%d/%d)\nActual Win %%: %f (%d/%d)\n\n", numTrials, ((float)riverExpectedWin / (float)(riverWin + riverLose)), riverExpectedWin, numTrials, ((float)(riverWin) / (float)(riverWin + riverLose)), riverWin, numTrials);
		fprintf(btout, "6 Card Results (%d hands)\nExpected Win %%: %f (%d/%d)\nActual Win %%: %f (%d/%d)\n\n", numTrials, ((float)turnExpectedWin / (float)(turnWin + turnLose)), turnExpectedWin, numTrials, ((float)(turnWin) / (float)(turnWin + turnLose)), turnWin, numTrials);
		fprintf(btout, "5 Card Results (%d hands)\nExpected Win %%: %f (%d/%d)\nActual Win %%: %f (%d/%d)\n\n", numTrials, ((float)flopExpectedWin / (float)(flopWin + flopLose)), flopExpectedWin, numTrials, ((float)(flopWin) / (float)(flopWin + flopLose)), flopWin, numTrials);

		
		
		fprintf(btout, "\nTraining Time: %lf seconds", elapsedSeconds);
		fclose(btout);
		printf("Training Time: %lf seconds\n", elapsedSeconds);
		printf("\nExpanded results saved to bulktest-results.txt\n");
		return 0;

	}
	
	
}

void bulkTest7(int numTrials, float winOdds) {
		
	//FILE * cardLogs7 = fopen("cardLogs7.csv", "w");
	//fprintf(cardLogs7, "pCards[0], pCards[1], cCards[0], cCards[1], cCards[2], cCards[3], cCards[4], score, odds\n");
	for(int trial = 1; trial <= numTrials; trial++) {
			
		int* hand = generateRandomHand(5);
		int pCards[2] = {hand[0], hand[1]};
		int cCards[5] = {hand[2], hand[3], hand[4], hand[5], hand[6]};

		int score = lookupHand(pCards, cCards);
		float odds = calcOdds(pCards, cCards);
			
		//fprintf(cardLogs7, "%s,%s,%s,%s,%s,%s,%s,%d,%f\n", cardNames[pCards[0]], cardNames[pCards[1]], cardNames[cCards[0]], cardNames[cCards[1]], cardNames[cCards[2]], cardNames[cCards[3]], cardNames[cCards[4]], score, odds);

		bool expectedWin;
		if(odds > winOdds)
			expectedWin = true;
		else
			expectedWin = false;

		bool cardFound = false;

		while(!cardFound) {
			int card = 1 + rand() % ((52 + 1) - 1);

			if(possibleCard(card, pCards, cCards)) {
				hand[0] = card;
				cardFound = true;
			}
		}
			
		cardFound = false;

		while(!cardFound) {
			int card = 1 + rand() % ((52 + 1) - 1);

			if(possibleCard(card, pCards, cCards)) {
				hand[1] = card;
				cardFound = true;
			}
		}
			
			
		if(expectedWin)
			riverExpectedWin++;
		
		int opponentScore = lookupHand(hand);

		if(opponentScore > score) {
			riverLose++;
		}
		else {
			riverWin++;
		}
			

		free(hand);
	}
	//fclose(cardLogs7);

}

void bulkTest6(int numTrials, float winOdds) {
	
	//FILE * cardLogs6 = fopen("cardLogs6.csv", "w");
	//fprintf(cardLogs6, "pCards[0], pCards[1], cCards[0], cCards[1], cCards[2], cCards[3], cCards[4], score, odds\n");
	for(int trial = 1; trial <= numTrials; trial++) {
		
		int* hand = generateRandomHand(5);
		int pCards[2] = {hand[0], hand[1]};
		int cCards[5] = {hand[2], hand[3], hand[4], hand[5], 0};

		float odds = calcOdds(pCards, cCards);

		//fprintf(cardLogs6, "%s,%s,%s,%s,%s,%s,%s,N/A,%f\n", cardNames[pCards[0]], cardNames[pCards[1]], cardNames[cCards[0]], cardNames[cCards[1]], cardNames[cCards[2]], cardNames[cCards[3]], cardNames[cCards[4]], odds);


		bool expectedWin;
		if(odds > winOdds)
			expectedWin = true;
		else
			expectedWin = false;


		cCards[4] = hand[6]; // add back the final card (river)

		int score = lookupHand(pCards, cCards);

		bool cardFound = false;

		while(!cardFound) {
			int card = 1 + rand() % ((52 + 1) - 1);

			if(possibleCard(card, pCards, cCards)) {
				hand[0] = card;
				cardFound = true;
			}
		}
			
		cardFound = false;

		while(!cardFound) {
			int card = 1 + rand() % ((52 + 1) - 1);

			if(possibleCard(card, pCards, cCards)) {
				hand[1] = card;
				cardFound = true;
			}
		}
			
			
		if(expectedWin)
			turnExpectedWin++;
		
		int opponentScore = lookupHand(hand);

		if(opponentScore > score) {
			turnLose++;
		}
		else {
			turnWin++;
		}

		free(hand);
	}
	
	//fclose(cardLogs6);

}

void bulkTest5(int numTrials, float winOdds) {
	
	//FILE * cardLogs5 = fopen("cardLogs5.csv", "w");
	//fprintf(cardLogs5, "pCards[0], pCards[1], cCards[0], cCards[1], cCards[2], cCards[3], cCards[4], score, odds\n");
	for(int trial = 1; trial <= numTrials; trial++) {
			
		int* hand = generateRandomHand(5);
		int pCards[2] = {hand[0], hand[1]};
		int cCards[5] = {hand[2], hand[3], hand[4], 0, 0};

		float odds = calcOdds(pCards, cCards);
			
		//fprintf(cardLogs5, "%s,%s,%s,%s,%s,%s,%s,N/A,%f\n", cardNames[pCards[0]], cardNames[pCards[1]], cardNames[cCards[0]], cardNames[cCards[1]], cardNames[cCards[2]], cardNames[cCards[3]], cardNames[cCards[4]], odds);

			
		bool expectedWin;
		if(odds > winOdds)
			expectedWin = true;
		else
			expectedWin = false;


		cCards[3] = hand[5]; // add back turn and river
		cCards[4] = hand[6];

		int score = lookupHand(pCards, cCards);

		bool cardFound = false;

		while(!cardFound) {
			int card = 1 + rand() % ((52 + 1) - 1);

			if(possibleCard(card, pCards, cCards)) {
				hand[0] = card;
				cardFound = true;
			}
		}
			
		cardFound = false;

		while(!cardFound) {
			int card = 1 + rand() % ((52 + 1) - 1);

			if(possibleCard(card, pCards, cCards)) {
				hand[1] = card;
				cardFound = true;
			}
		}
			
			
		if(expectedWin)
			flopExpectedWin++;
		
		int opponentScore = lookupHand(hand);

		if(opponentScore > score) {
			flopLose++;
		}
		else {
			flopWin++;
		}


		free(hand);
	}
	//fclose(cardLogs5);

}


// Generates unique, random cards (player + community)
// int numCards is the number of KNOWN cards we want
// so 3 = flop, 4 = turn, 5 = river
int* generateRandomHand(int numCards) {
	// USING Knuth Algorithm
	int* hand = (int*)malloc(sizeof(int) * 7);
	memset(hand, 0, sizeof(hand));
		
	int in, im;

	im = 0;

	for (in = 0; in < 53 && im < 7; ++in) {
		int rn = 52 - in;
		int rm = 7 - im;
		if (rand() % rn < rm)    
    			/* Take it */
			hand[im++] = in + 1; /* +1 since your range begins from 1 */
	}


	// Randomize our "increasing random generation"
	// https://stackoverflow.com/questions/20734774/random-array-generation-with-no-duplicates
	for(int i = 0; i < 7; i++) {
		int j = i + rand() / (RAND_MAX / (7 - i) + 1);
		int t = hand[j];
		hand[j] = hand[i];
		hand[i] = t;
	}

	for(int i = (2 + numCards); i < 7; i++) {
		hand[i] = 0; // plug in uknown cards
	}
	
	return hand;
}

// checks size of cCards and returns from proper odds function
float calcOdds(int* pCards, int* cCards) {
	int cCardsCount = 0;
	while(cCardsCount < 5 && cCards[cCardsCount] != 0) {
		cCardsCount++;
	}

	if(cCardsCount == 3)
		return calcOdds5(pCards, cCards);
	else if(cCardsCount == 4)
		return calcOdds6(pCards, cCards);
	else if(cCardsCount == 5)
		return calcOdds7(lookupHand(pCards, cCards), pCards, cCards);
	else
		return -1;

}



float calcAvgScore(int* pCards, int* cCards) {
	int cCardsCount = 0;
	while(cCardsCount < 5 && cCards[cCardsCount] != 0) {
		cCardsCount++;
	}
		
	if(cCardsCount == 3)
		return avgScore5(pCards, cCards);
	else if(cCardsCount == 4)
		return avgScore6(pCards, cCards);
	else if(cCardsCount == 5)
		return lookupHand(pCards, cCards);
	else
		return -1;
}


// Prints the player hand in the following format:
// (plyCard1 plyCard2) communityCard1 communityCard2...
// Unknown communityCards are printed as ??
//
// Parameters
//	int* pCards - Array of player cards (values/ints), length must equal 2
//	int* cCards - Array of community cards (value/ints), length must equal 5 (unknown cards be 0)
void printHand(int* pCards, int* cCards) {
	
	// player cards
	printf("Player Hand: (%s %s) ", cardNames[pCards[0]], cardNames[pCards[1]]);
	
	// community cards
	for(int i = 0; i < 5; i++) {
		printf("%s ", cardNames[cCards[i]]);
	}
	printf("\n");
}

// Prints the player hand in the following format:
// (plyCard1 plyCard2) communityCard1 communityCard2...
// Unknown communityCards are printed as ??
//
// Parameters
//	int* hand - Array of player cards and community cards, length must be 7 (unknown cards be 0), player cards must be idx 0 and 1
void printHand(int* hand) {
	printf("Player Hand: (%s %s) %s %s %s %s %s\n", cardNames[hand[0]], cardNames[hand[1]], cardNames[hand[2]], cardNames[hand[3]], cardNames[hand[4]], cardNames[hand[5]], cardNames[hand[6]]);
}


// Combines player and community cards into a single array called the hand
//
// Parameters
//	int* pCards - Array of player cards (must be size 2)
//	int* cCards - Array of community cards (must be size 5), unknown cards should be 0
// Returns
//	int* hand - Array of size 7 with pCards at idx 0 and 1, cCards for 3-6
int* combinePlyCommunity (int* pCards, int* cCards) {

	// ply, ply, com, com, com, com, com
	int* pokerHand = (int*)malloc(sizeof(int) * 7);
	memset(pokerHand, 0, sizeof(pokerHand));

	pokerHand[0] = pCards[0];
	pokerHand[1] = pCards[1];


	// add community cards
	for(int i = 0; i <= 4; i++) {
		pokerHand[i+2] = cCards[i];
	}

	return pokerHand;

}

// Returns integer value of given cardName (<value><suit>) (Ex: 2c = 2 of clubs, th = 10 of hearts, qs = queen of spades)
// Parameters
//	char* cardName - Card name in the format specificed above, must be size 2
// Returns
//	int cardVal - Integer card value for the given cardName
int cardNametoInt(char* cardName) {
	
	for(int i = 0; i <= 52; i++) {
		if(strcmp(cardNames[i], cardName) == 0)	
			return i;
	}

	return 0;
}

// Looks up and returns the score of a 5/6/7-card hand in HandRanks.dat
// Returns only the current score, not projected for 5 and 6 card
// Slightly modified version of TwoPlusTwoHandEvaluator code
//
// Parameters
//	int* hand - Array of player cards and community cards, length must be 7 (unknown cards be 0), player cards must be idx 0 and 1
// Returns
//	int score - Computed score found in HandRanks.dat for a given hand (current score)
int lookupHand(int* pHand) {
	
	int p = HR[53 + *pHand++];
	p = HR[p + *pHand++];
	p = HR[p + *pHand++];
	p = HR[p + *pHand++];
	p = HR[p + *pHand++];
	if(*pHand == 0) { // pass 0 only once (5 card/flop analysis)
		return HR[p + *pHand++];
	}
	else { // 6 or 7 card, pass 0 once or the river
		p = HR[p + *pHand++];
		return HR[p + *pHand++];
	}
}

// Looks up and returns the score of a 5/6/7-card hand in HandRanks.dat
// Returns only the current score, not projected for 5 and 6 card
// Slightly modified version of TwoPlusTwoHandEvaluator code
//
// Parameters
// 	int* pCards - Array of playerCards, must be of length 2
// 	int* cCards - Array of communityCards, must be of length 5, unknown cards as 0
// Returns
//
//	int score - Computed score found in HandRanks.dat for a given hand (current score)
int lookupHand(int* pCards, int* cCards) {
	
	int p = HR[53 + pCards[0]];
	p = HR[p + pCards[1]];
	p = HR[p + cCards[0]];
	p = HR[p + cCards[1]];
	p = HR[p + cCards[2]];
	if(cCards[3] == 0) { // pass 0 only once (5 card/flop analysis)
		return HR[p + cCards[3]];
	}
	else { // 6 or 7 card, pass 0 once or the river
		p = HR[p + cCards[3]];
		return HR[p + cCards[4]];
	}
}

// Simply returns false if the card exists on the table or in your hand
//
// Parameters
//	int card - Card value to check whether it exists in pCards or cCards
//	int* pCards - Array of player cards to search through
//	int* cCards - Array of community cards to search through
// Returns
//	bool possibleCard - Returns true if the card doesn't exist in pCards or cCards, otherwise false
bool possibleCard(int card, int* pCards, int* cCards) {
	if( (card == pCards[0]) || (card == pCards[1]) || (card == cCards[0]) || (card == cCards[1]) || (card == cCards[2]) || (card == cCards[3]) || (card == cCards[4]) || (card == cCards[5]) )
		return false;

	return true;
}

// Calculates the % chance of winning by computing the quantile your hand is at versus all possible opponent hands
// Parameters
//	int score - score to compare to
//	int* pCards - Array of player cards
//	int* cCards - Array of community cards
// Returns
//	float odds - % chance of winning the hand / quantile of hand strength
float calcOdds7(int score, int* pCards, int* cCards) {
	int opponentCards[2] = {0, 0};

	int worseHands = 0;
	int betterHands = 0;

	// This loop checks the score of all possible combinations of opponent
	// cards given the 5 community cards, and computers the number of hands
	// better and worse than the passed in score
	for(int i = 1; i <= 52; i++) { // gets first card
		if(!possibleCard(i, pCards, cCards))	
			continue;
		
		for(int j = i+1; j <= 52; j++) { // gets second card
			if(!possibleCard(j, pCards, cCards))	
				continue;
			
			opponentCards[0] = i;
			opponentCards[1] = j;
		
			int opponentScore = lookupHand(opponentCards, cCards);

			if(opponentScore > score) {
				betterHands++;
			}
			else {
				worseHands++;
			}
		}
	}

	return (float)( (float)worseHands / (float)(worseHands + betterHands) );

}


// Calculates the % chance of winning by computing the quantile your hand is at versus all possible opponent hands
// With 6 hand, we add an additional layer (for loop) to compare scores with all possible rivers
// Parameters
//	int* pCards - Array of player cards
//	int* cCards - Array of community cards
// Returns
//	float odds - % chance of winning the hand / quantile of hand strength
float calcOdds6(int* pCards, int* cCards) {
	int opponentCards[2] = {0, 0};

	int worseHands = 0;
	int betterHands = 0;

	// This loop checks the score of all possible combinations of opponent
	// cards given the 5 community cards, and computers the number of hands
	// better and worse than the passed in score
	for(int i = 1; i <= 52; i++) { // gets first card
		if(!possibleCard(i, pCards, cCards))	
			continue;
		
		for(int j = i+1; j <= 52; j++) { // gets second card
			if(!possibleCard(j, pCards, cCards))
				continue;
			
			opponentCards[0] = i;
			opponentCards[1] = j;

			for(int k = 1; k <= 52; k++) {
				if( (!possibleCard(k, opponentCards, cCards)) || (!possibleCard(k, pCards, cCards)) ) {
					continue;
				}

				cCards[4] = k;
				int opponentScore = lookupHand(opponentCards, cCards);
				int playerScore = lookupHand(pCards, cCards);

				if(opponentScore > playerScore) {
					betterHands++;
				}
				else {
					worseHands++;
				}
				
				cCards[4] = 0; // reset altered card

			}
		

		}
	}


	return (float)( (float)worseHands / (float)(worseHands + betterHands) );

}

// Calculates the % chance of winning by computing the quantile your hand is at versus all possible opponent hands
// With 5 hand, we add 2 additional layers (for loops) to compare scores with all possible turns and rivers
// Parameters
//	int* pCards - Array of player cards
//	int* cCards - Array of community cards
// Returns
//	float odds - % chance of winning the hand / quantile of hand strength
float calcOdds5(int* pCards, int* cCards) {

	int opponentCards[2] = {0, 0};

	int worseHands = 0;
	int betterHands = 0;

	// This loop checks the score of all possible combinations of opponent
	// cards given the 5 community cards, and computers the number of hands
	// better and worse than the passed in score
	for(int i = 1; i <= 52; i++) { // gets first card
		if(!possibleCard(i, pCards, cCards))	
			continue;
		
		for(int j = i+1; j <= 52; j++) { // gets second card
			if(!possibleCard(j, pCards, cCards))	
				continue;
			
			opponentCards[0] = i;
			opponentCards[1] = j;

			for(int k = 1; k <= 52; k++) {
				if( (!possibleCard(k, opponentCards, cCards)) || (!possibleCard(k, pCards, cCards)) ) {
					continue;
				}


				for(int v = k+1; v <= 52; v++) {
					
					if( (!possibleCard(v, opponentCards, cCards)) || (!possibleCard(v, pCards, cCards)) ) {
						continue;
					}
				
					cCards[3] = k;	
					cCards[4] = v;

					int opponentScore = lookupHand(opponentCards, cCards);
					int playerScore = lookupHand(pCards, cCards);
			
					if(opponentScore > playerScore) {
						betterHands++;
					}
					else {
						worseHands++;
					}

					cCards[3] = 0;
					cCards[4] = 0; // reset
	
				}

			
			}
		

		}
	}

	return (float)( (float)worseHands / (float)(worseHands + betterHands) );

}


// Computes average possible score of a hand given 6 cards by taking average score of all possible river outcomes
// Parameters
//	int* pCards - Array of player cards (must be size 2)
//	int* cCards - Array of community cards (must be size 5), cCards[4] is ignored
// Returns
//	float avgScore - Average score possible on all possible rivers
float avgScore6(int* pCards, int* cCards) {

	float avgScore = 0;
	int count = 0;
	// this loop will get every possible card (of the remaining 52 - 6 = 46) and compute an average score
	for(int i = 1; i <= 52; i++) {
		if(!possibleCard(i, pCards, cCards)) {
			continue; // card is already on the table
		}

		cCards[4] = i;
		avgScore += lookupHand(pCards, cCards);
		count++;
	
		cCards[4] = 0; // reset altered value	
	
	}

	return (float)avgScore / (float)count; // dividing by 46 since thats the remaining number of cards to check
}

// Computes average possible score of a hand given 5 cards by taking average score of all possible turn and river outcomes
// Parameters
//	int* pCards - Array of player cards (must be size 2)
//	int* cCards - Array of community cards (must be size 5), cCards[3] and cCards[4] is ignored
// Returns
//	float avgScore - Average score possible on all possible turns and rivers
float avgScore5(int* pCards, int* cCards) {

	float avgScore = 0;
	int count = 0;
	// this loop will get every possible card (of the remaining 52 - 6 = 46) and compute an average score
	for(int i = 1; i <= 52; i++) {
		if(!possibleCard(i, pCards, cCards)) {
			continue; // card is already on the table
		}

		for(int j = i+1; j <= 52; j++) {
			if(!possibleCard(j, pCards, cCards))
				continue;

			cCards[3] = i;
			cCards[4] = j;

			avgScore += lookupHand(pCards, cCards);
			count++;
		
			cCards[3] = 0;
			cCards[4] = 0;
		}
	
	}

	return (float)avgScore / (float)count;
}

