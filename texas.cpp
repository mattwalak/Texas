#include <math.h>
#include <string.h>
#include <iostream>
#include <stdio.h> 
#include <stdlib.h>
#include <vector>
#include "MERSENNE_TWISTER.h"

MERSENNE_TWISTER randy;

#define SUIT(c) c%4
#define VALUE(c) c/4
#define CARD(v,s) v*4 + s

using namespace std;

int numTies = 0;

string ctos(int c){
	string result = "";
	switch(VALUE(c)){
		case 0:
			result += "2 of ";
			break;
		case 1:
			result += "3 of ";
			break;
		case 2:
			result += "4 of ";
			break;
		case 3:
			result += "5 of ";
			break;
		case 4:
			result += "6 of ";
			break;
		case 5:
			result += "7 of ";
			break;
		case 6:
			result += "8 of ";
			break;
		case 7:
			result += "9 of ";
			break;
		case 8:
			result += "10 of ";
			break;
		case 9:
			result += "J of ";
			break;
		case 10:
			result += "Q of ";
			break;
		case 11:
			result += "K of ";
			break;
		case 12:
			result += "A of ";
			break;
		default:
			return "invalid card";
	}

	switch(SUIT(c)){
		case 0:
			result += "clubs";
			break;
		case 1:
			result += "diamonds";
			break;
		case 2:
			result += "hearts";
			break;
		case 3:
			result += "spades";
			break;
		default:
			return "invalid card";
	}
	return result;
}

string handToStr(int * hand, int len){
	string result = "";
	for(int i = 0; i < len; i++){
		result += ctos(hand[i]) + "; ";
	}
	return result;
}

void isort(int * hand, int len){
	for(int i = 0; i < len; i++){
		int maxSoFar = -1;
		int maxIndex = -1;
		for(int j = i; j < len; j++){
			if(hand[j] > maxSoFar){
				maxSoFar = hand[j];
				maxIndex = j;
			}
		}
		int temp = hand[i];
		hand[i] = hand[maxIndex];
		hand[maxIndex] = temp;
	}
}

// Assumes hand is sorted highest to lowest
bool hasStraight(int * hand, int len){
	for(int i = 0; i < len-1; i++)
		if(VALUE(hand[i])-VALUE(hand[i+1]) != 1)
			return false;
	return true;
}

bool hasFlush(int * hand, int len){
	int suit = SUIT(hand[0]);
	for(int i = 1; i < len; i++)
		if(SUIT(hand[i]) != suit) 
			return false;
	return true;
}

// Bool returns: {hasFourKind, hasThreeKind, hasTwoPair, hasOnePair}
// Tiebreak contains the cards we resort to in the case of a tie
void findGroupings(int * hand, int len, bool * results){
	for(int i = 0; i < 4; i++) results[i] = false;

	int counts[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	for(int i = 0; i < len; i++)
		counts[VALUE(hand[i])]++;
	for(int i = 0; i < 13; i++){
		if(counts[i] == 4){
			results[0] = true;
		}else if(counts[i] == 3){
			results[1] = true;
		}else if(counts[i] == 2){
			if(results[3])
				results[2] = true; // two pair
			else
				results[3] = true;
		}		
	}
}

/*
0 = high card
1 = one pair
2 = two pair
3 = three of a kind
4 = straight
5 = flush
6 = full house
7 = four of a kind
8 = straight flush
9 = royal (straight) flush

Assumes hand is sorted greatest to least
*/
int getRank(int * hand, int len){
	bool straight = hasStraight(hand, len);
	bool flush = hasFlush(hand, len);
	bool groups[4];
	findGroupings(hand, len, groups);
	if(straight && flush){
		if(VALUE(hand[0]) == 12)
			return 9;
		else
			return 8;
	}
	if(groups[0])
		return 7;
	if(groups[1] && groups[3])
		return 6;
	if(flush)
		return 5;
	if(straight)
		return 4;
	if(groups[1])
		return 3;
	if(groups[2])
		return 2;
	if(groups[3])
		return 1;
	return 0; // Your hand sucks by the way
}

// Simple by value comparison
// Compare highest card, next highest card, next highest card
// Final tie break is suit of highest card
// Assumes hands are sorted greatest to least
// Returns true if hand_1 is greatest, false if hand_2 is
bool valueCompare(int * hand_1, int * hand_2, int len){
	for(int i = 0; i < len; i++){
		int value_1 = VALUE(hand_1[i]);
		int value_2 = VALUE(hand_2[i]);
		if(value_1 > value_2)
			return true;
		else if (value_2 > value_1)
			return false;
	}
	return (SUIT(hand_1[0]) > SUIT(hand_2[0]));
}

// Returns true if hand_1 wins in a tie break situation for a given rank
// returns false if hand_2 wins
// Assumes hands are sorted greatest to least
// There will never be two identical hands so just shut up about it would you
bool tieBreak(int * hand_1, int * hand_2, int len, int rank){
	if(rank == 9){
		// Just go by alphabetical suit rankings backwards (Spades is highest)
		// Technically should be a tie here
		return (SUIT(hand_1[0]) > SUIT(hand_2[0]));
	}
	if(rank == 8){
		// Compare highest card (Because this is texas holdem, we cant have two straight
		// flushes with the same strength in a different suit)
		// but whatever, if that happens determine winner by most powerful suit
		int value_1 = VALUE(hand_1[0]);
		int value_2 = VALUE(hand_2[0]);
		if(value_1 > value_2)
			return true;
		else if(value_2 > value_1)
			return false;
		else
			return (SUIT(hand_1[0]) > SUIT(hand_2[0]));
	}
	if(rank == 7){
		// Use value of four pair (in tiebreaker array)
		// No, we can't have competing four of a kind... thats stupid
		int c0h1 = VALUE(hand_1[0]);
		int c1h1 = VALUE(hand_1[1]);
		int value_1 = (c0h1 == c1h1 ? c0h1 : c1h1);
		int c0h2 = VALUE(hand_2[0]);
		int c1h2 = VALUE(hand_2[1]);
		int value_2 = (c0h2 == c1h2 ? c0h2 : c1h2);
		return (value_1 > value_2);
	}
	if(rank == 6){
		// Use value of three pair (in tiebreaker array)
		// can't have two equal three pairs, obviously
		int c1h1 = VALUE(hand_1[1]);
		int c2h1 = VALUE(hand_1[2]);
		int value_1 = (c1h1 == c2h1 ? VALUE(hand_1[0]) : VALUE(hand_1[2]));
		int c1h2 = VALUE(hand_2[1]);
		int c2h2 = VALUE(hand_2[2]);
		int value_2 = (c1h2 == c2h2 ? VALUE(hand_2[0]) : VALUE(hand_2[2]));
		return (value_1 > value_2);
	}
	if(rank == 5){
		// Simple by value comparison	
		return valueCompare(hand_1, hand_2, len);
	}
	if(rank == 4){
		// Compare highest card
		// Problem here is you can actually end up in a tie...
		// For now, we'll go by suit rankings
		int value_1 = VALUE(hand_1[0]);
		int value_2 = VALUE(hand_2[0]);
		if(value_1 > value_2)
			return true;
		else if(value_2 > value_1)
			return false;
		else
			return (SUIT(hand_1[0]) > SUIT(hand_2[0]));
	}
	if(rank == 3){
		// Compare value of three of a kind (in tie breaker array)
		// Can't have matching three pair silly
		int value_1;
		if(VALUE(hand_1[0]) == VALUE(hand_1[2]))
			value_1 = VALUE(hand_1[0]);
		else if(VALUE(hand_1[1]) == VALUE(hand_1[3]))
			value_1 = VALUE(hand_1[1]);
		else
			value_1 = VALUE(hand_1[2]);
		int value_2;
		if(VALUE(hand_2[0]) == VALUE(hand_2[2]))
			value_2 = VALUE(hand_2[0]);
		else if(VALUE(hand_2[1]) == VALUE(hand_2[3]))
			value_2 = VALUE(hand_2[1]);
		else
			value_2 = VALUE(hand_2[2]);
		return (value_1 > value_2);
	}
	if(rank == 2){
		// Compare values of highest pair, lowest pair, then 5th card (in tie breaker array)
		// If still tied, compare suit of highest card
		int h1_pair1;
		int h1_pair2;
		int h1_kick;
		if(VALUE(hand_1[1]) == VALUE(hand_1[2])){
			h1_kick = VALUE(hand_1[0]);
			h1_pair1 = VALUE(hand_1[1]);
			h1_pair2 = VALUE(hand_1[3]);
		}else if(VALUE(hand_1[2]) == VALUE(hand_1[3])){
			h1_kick = VALUE(hand_1[4]);
			h1_pair1 = VALUE(hand_1[0]);
			h1_pair2 = VALUE(hand_1[2]);
		}else{
			h1_kick = VALUE(hand_1[2]);
			h1_pair1 = VALUE(hand_1[0]);
			h1_pair2 = VALUE(hand_1[3]);
		}
		int h2_pair1;
		int h2_pair2;
		int h2_kick;
		if(VALUE(hand_2[1]) == VALUE(hand_2[2])){
			h2_kick = VALUE(hand_2[0]);
			h2_pair1 = VALUE(hand_2[1]);
			h2_pair2 = VALUE(hand_2[3]);
		}else if(VALUE(hand_2[2]) == VALUE(hand_2[3])){
			h2_kick = VALUE(hand_2[4]);
			h2_pair1 = VALUE(hand_2[0]);
			h2_pair2 = VALUE(hand_2[2]);
		}else{
			h2_kick = VALUE(hand_2[2]);
			h2_pair1 = VALUE(hand_2[0]);
			h2_pair2 = VALUE(hand_2[3]);
		}

		if(h1_pair1 > h2_pair1)
			return true;
		else if(h2_pair1 > h1_pair1)
			return false;

		if(h1_pair2 > h2_pair2)
			return true;
		else if(h2_pair2 > h1_pair2)
			return false;

		if(h1_kick > h2_kick)
			return true;
		else if(h2_kick > h1_kick)
			return false;

		return (SUIT(hand_1[0]) > SUIT(hand_2[0]));
	}
	if(rank == 1){
		// Compare values of pair (in tie breaker array)
		// If tied, compare remaining three cards (also in tie breaker)
		// If still tied, compare suit of highest card
		int h1_pair;
		int h1_kick1;
		int h1_kick2;
		int h1_kick3;
		if(VALUE(hand_1[0]) == VALUE(hand_1[1])){
			h1_pair = VALUE(hand_1[0]);
			h1_kick1 = VALUE(hand_1[2]);
			h1_kick2 = VALUE(hand_1[3]);
			h1_kick3 = VALUE(hand_1[4]);
		}else if(VALUE(hand_1[1]) == VALUE(hand_1[2])){
			h1_pair = VALUE(hand_1[1]);
			h1_kick1 = VALUE(hand_1[0]);
			h1_kick2 = VALUE(hand_1[3]);
			h1_kick3 = VALUE(hand_1[4]);
		}else if(VALUE(hand_1[2]) == VALUE(hand_1[3])){
			h1_pair = VALUE(hand_1[2]);
			h1_kick1 = VALUE(hand_1[0]);
			h1_kick2 = VALUE(hand_1[1]);
			h1_kick3 = VALUE(hand_1[4]);
		}else{
			h1_pair = VALUE(hand_1[3]);
			h1_kick1 = VALUE(hand_1[0]);
			h1_kick2 = VALUE(hand_1[1]);
			h1_kick3 = VALUE(hand_1[2]);
		}
		int h2_pair;
		int h2_kick1;
		int h2_kick2;
		int h2_kick3;
		if(VALUE(hand_2[0]) == VALUE(hand_2[1])){
			h2_pair = VALUE(hand_2[0]);
			h2_kick1 = VALUE(hand_2[2]);
			h2_kick2 = VALUE(hand_2[3]);
			h2_kick3 = VALUE(hand_2[4]);
		}else if(VALUE(hand_2[1]) == VALUE(hand_2[2])){
			h2_pair = VALUE(hand_2[1]);
			h2_kick1 = VALUE(hand_2[0]);
			h2_kick2 = VALUE(hand_2[3]);
			h2_kick3 = VALUE(hand_2[4]);
		}else if(VALUE(hand_2[2]) == VALUE(hand_2[3])){
			h2_pair = VALUE(hand_2[2]);
			h2_kick1 = VALUE(hand_2[0]);
			h2_kick2 = VALUE(hand_2[1]);
			h2_kick3 = VALUE(hand_2[4]);
		}else{
			h2_pair = VALUE(hand_2[3]);
			h2_kick1 = VALUE(hand_2[0]);
			h2_kick2 = VALUE(hand_2[1]);
			h2_kick3 = VALUE(hand_2[2]);
		}

		if(h1_pair > h2_pair)
			return true;
		else if(h2_pair > h1_pair)
			return false;

		if(h1_kick1 > h2_kick1)
			return true;
		else if(h2_kick1 > h1_kick1)
			return false;

		if(h1_kick2 > h2_kick2)
			return true;
		else if(h2_kick2 > h1_kick2)
			return false;

		if(h1_kick3 > h2_kick3)
			return true;
		else if(h2_kick3 > h1_kick3)
			return false;

		return (SUIT(hand_1[0]) > SUIT(hand_2[0]));
	}
	if(rank == 0){
		// Simple by value comparison
		
		return valueCompare(hand_1, hand_2, len);
	}

	cout << "Something went wrong here... :(" << endl;
	return true;
}

// Returns true if hand_1 wins, false if hand_2 wins
// Assumes hands are sorted from highest to lowest
bool compare(int * hand_1, int * hand_2, int len){
	int rank_1 = getRank(hand_1, len);
	int rank_2 = getRank(hand_2, len);
	if(rank_1 > rank_2){
		return true;
	}else if(rank_1 < rank_2){
		return false;
	}else{
		return tieBreak(hand_1, hand_2, len, rank_1);
	}
}

// Assumes a 5 card hand (int * must be allocated before)
void intToHand(int id, int * out){
	for(int i = 0; i < 5; i++){
		out[i] = id%52;
		id /= 52;
	}
}

int stoc(char * in){
	// Find value;
	int val = atoi(in);
	if(val == 0){
		switch(in[0]){
			case 'j':
				val = 11;
				break;
			case 'q':
				val = 12;
				break;
			case 'k':
				val = 13;
				break;
			case 'a':
				val = 14;
				break;
			default:
				cout << "INVALID INPUT: " << in << endl;
				exit(0);
		}
	}
	val -= 2;

	// Find suit
	int suit;
	int i = 1;
	while(isdigit(in[i])) 
		i++;
	switch(in[i]){
		case 'c':
			suit = 0;
			break;
		case 'd':
			suit = 1;
			break;
		case 'h':
			suit = 2;
			break;
		case 's':
			suit = 3;
			break;
		default:
			cout << "INVALID INPUT: " << in << endl;
			exit(0);
	}

	return CARD(val, suit);
}

// Deals two hands of 5 cards from a shuffled deck
void twoHandsFromDeck(int * hand_1, int * hand_2){
	vector<int> deck;
	for(int i = 0; i < 52; i++)
		deck.push_back(i);
	for(int i = 0; i < 10; i++){
		int r = floor(randy.rand() * deck.size()); // Make sure no off by one here
		if(i%2 == 0)
			hand_1[i/2] = deck[r];
		else
			hand_2[i/2] = deck[r];

		deck.erase(deck.begin()+r);
	}
}

// Arg list
int main(int argc, char ** argv){
	
	if(argc < 10){
		cout << "fine i'll be nice about it. enter 10 cards. please." << endl;
	}

	int hand_1[5];
	int hand_2[5];
	//twoHandsFromDeck(hand_1, hand_2);
	for(int i = 1; i < 6; i++){
		hand_1[i-1] = stoc(argv[i]);
	}
	for(int i = 6; i < 11; i++){
		hand_2[i-6] = stoc(argv[i]);
	}
	isort(hand_1, 5);
	isort(hand_2, 5);

	cout << "hand_1:\n" << handToStr(hand_1, 5) << "\nhand_2:\n" << handToStr(hand_2, 5) << endl;
	cout << (compare(hand_1, hand_2, 5) ? "hand_1 wins" : "hand_2 wins") << endl;



	/*if(argc < 3){
		cout << "you clearly don't know what you're doing" << endl;
		exit(0);
	}*/

	/*
	int max = pow(52, 2) - 1;
	int win = 0;
	int loss = 0;

	for(int i = 0; i < 100000000; i++){
		int hand_1[5];
		int hand_2[5];
		twoHandsFromDeck(hand_1, hand_2);
		if(compare(hand_1, hand_2, 5)){
			win++;
		}else{
			loss++;
		}

		if(i%1000000 == 0)
			cout << i/1000000 << endl;
	}

	cout << "wins: " << win << "; loss: " << loss << endl;
	cout << "exact ties = " << numTies << endl;*/

	/*
	int * hand_1 = new int[5];
	hand_1[0] = atoi(argv[1]);
	hand_1[1] = atoi(argv[2]);
	hand_1[2] = atoi(argv[3]);
	hand_1[3] = atoi(argv[4]);
	hand_1[4] = atoi(argv[5]);
	isort(hand_1, 5);

	int * hand_2 = new int[5];
	hand_2[0] = atoi(argv[6]);
	hand_2[1] = atoi(argv[7]);
	hand_2[2] = atoi(argv[8]);
	hand_2[3] = atoi(argv[9]);
	hand_2[4] = atoi(argv[10]);
	isort(hand_2, 5);

	cout << "hand_1:\n" << handToStr(hand_1, 5) << "\nhand_2:\n" << handToStr(hand_2, 5) << endl;
	cout << (compare(hand_1, hand_2, 5) ? "hand_1 wins" : "hand_2 wins") << endl;
	*/
}