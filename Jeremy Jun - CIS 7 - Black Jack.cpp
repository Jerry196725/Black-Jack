/*
Jeremy Jun
12/4/2024
CIS 7
final project: Black Jack
*/

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <map>
#include <numeric>
using namespace std;

class randomCard {
public:
    struct card {

        string value;
        int points;

    };

    vector<card> createCards() {

        vector<card> deck;
        vector<string> values = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King", "Ace" };
        vector<int> points = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 11 };

        for (size_t i = 0; i < values.size(); i++) {

            for (int j = 0; j < 4; j++) {

                deck.push_back({ values[i], points[i] });
            }
        }
        return deck;
    }

    void shuffleDeck(vector<card>& deck) {

        srand(time(0));

        for (size_t i = 0; i < deck.size(); i++) {

            swap(deck[i], deck[rand() % deck.size()]);
        }
    }

    map<int, int> countDeck(const vector<card>& deck) {
        map<int, int> cardCounts;
        for (const auto& card : deck) {
            cardCounts[card.points]++;
        }
        return cardCounts;
    }
};

// Score and Evaluation
class scoreBoard {

public:

    int calculateScore(const vector<randomCard::card>& hand) {

        int score = 0, aces = 0;

        for (const auto& c : hand) {

            score += c.points;
            if (c.value == "Ace") aces++;

        }

        while (score > 21 && aces > 0) {

            score -= 10;
            aces--;

        }
        return score;
    }

    void evaluateWinner(int playerScore, int dealerScore) {

        if (playerScore > 21) cout << "\nPlayer busts. Dealer wins!\n";
        else if (dealerScore > 21) cout << "\nDealer busts. Player wins!\n";
        else if (playerScore > dealerScore) cout << "\nPlayer wins!\n";
        else if (playerScore < dealerScore) cout << "\nDealer wins!\n";
        else cout << "\nIt's a push!\n";
    }
};

class dealer : public randomCard {

private:

    vector<card> hand;

public:

    void playTurn(vector<card>& deck, scoreBoard& sb) {

        hand.push_back(deck.back());
        deck.pop_back();
        hand.push_back(deck.back());
        deck.pop_back();

        while (sb.calculateScore(hand) < 17) {
            hand.push_back(deck.back());
            deck.pop_back();
        }
    }

    map<int, double> dealerProbabilities(const vector<card>& deck, scoreBoard& sb) {

        map<int, int> cardCounts = countDeck(deck);
        int totalCards = accumulate(cardCounts.begin(), cardCounts.end(), 0,
            [](int sum, const auto& pair) { return sum + pair.second; });

        map<int, double> dealerScores;
        vector<int> outcomes(22, 0);

        auto simulateDealer = [&](int score, int softAces, auto& self) -> void {
            if (score >= 17) {
                if (score > 21) outcomes[21]++;
                else outcomes[score]++;
                return;
            }
            for (auto& [value, count] : cardCounts) {
                if (count == 0) continue;
                cardCounts[value]--;

                int newScore = score + value;
                if (value == 11) softAces++;
                if (newScore > 21 && softAces > 0) {
                    newScore -= 10;
                    softAces--;
                }
                self(newScore, softAces, self);
                cardCounts[value]++;
            }
        };

        simulateDealer(0, 0, simulateDealer);

        for (int i = 0; i <= 21; ++i) {

            dealerScores[i] = static_cast<double>(outcomes[i]) / totalCards;

        }
        return dealerScores;
    }

    vector<card> getHand() const { return hand; }
};

class player : public dealer {

private:

    vector<randomCard::card> hand;

public:

    // Determines the player's chance of winning the game.
    double calculateWinProbability(const vector<randomCard::card>& hand,
        const vector<randomCard::card>& deck,
        scoreBoard& sb,
        dealer& dealerObj) {

        int playerScore = sb.calculateScore(hand);

        if (playerScore > 21) return 0.0;

        map<int, double> dealerScores = dealerObj.dealerProbabilities(deck, sb);

        double winProbability = 0.0;

        for (const auto& pair : dealerScores) {

            int dealerScore = pair.first;

            double prob = pair.second;    

            if (dealerScore > 21 || playerScore > dealerScore) {
                winProbability += prob;
            }
        }

        return winProbability;
    }

    void playTurn(vector<randomCard::card>& deck, scoreBoard& sb, dealer& dealerObj) {

        hand.push_back(deck.back());
        deck.pop_back();
        hand.push_back(deck.back());
        deck.pop_back();


        while (true) {

            cout << "\nYour Hand: ";
            for (const auto& card : hand) {

                cout << card.value << " ";
            }
            cout << "| Total Score: " << sb.calculateScore(hand) << "\n";

            int score = sb.calculateScore(hand);
            if (score > 21) {

                cout << "You busted!\n";
                break;
            }
            else if (score == 21) {

                cout << "\n**********\n";
                cout << "Blackjack!\n";
                cout << "**********\n";

                break;
            }

            double winProb = calculateWinProbability(deck, hand, sb, dealerObj);
            cout << "Winning Probability before hit: " << winProb * 100 << "%\n";

            cout << "Choose an action - Hit (h) or Stand (s): ";
            char choice;
            cin >> choice;

            if (choice == 'h') {

                hand.push_back(deck.back());
                deck.pop_back();
                double updatedProb = calculateWinProbability(deck, hand, sb, dealerObj);
                cout << "Winning Probability after hit: " << updatedProb * 100 << "%\n";
            }
            else if (choice == 's') {

                break;
            }
            else {

                cout << "Invalid choice. Please try again.\n";
            }
        }
    }


    vector<randomCard::card> getHand() const { return hand; }
};

int main() {

    randomCard randomCardObj;
    vector<randomCard::card> deck = randomCardObj.createCards();
    randomCardObj.shuffleDeck(deck);

    scoreBoard scoreBoardObj;
    player playerObj;
    dealer dealerObj;


    cout << "Player's turn:\n";
    playerObj.playTurn(deck, scoreBoardObj, dealerObj);

    cout << "\nDealer's turn:\n";
    dealerObj.playTurn(deck, scoreBoardObj);


    cout << "\nFinal Results:\n";
    cout << "Player Hand: ";
    for (const auto& card : playerObj.getHand()) {

        cout << card.value << " ";
    }

    cout << "| Total Score: " << scoreBoardObj.calculateScore(playerObj.getHand()) << "\n";

    cout << "Dealer Hand: ";
    for (const auto& card : dealerObj.getHand()) {

        cout << card.value << " ";
    }

    cout << "| Total Score: " << scoreBoardObj.calculateScore(dealerObj.getHand()) << "\n";

    scoreBoardObj.evaluateWinner(
        scoreBoardObj.calculateScore(playerObj.getHand()),
        scoreBoardObj.calculateScore(dealerObj.getHand())
    );

    return 0;
}

