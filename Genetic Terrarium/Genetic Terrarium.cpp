// GeneticTerrarium.cpp : Defines the entry point for the console application.
//

/*	c++ graphical game design tutorial with sfml http://www.gamefromscratch.com/page/Game-From-Scratch-CPP-Edition-The-Introduction.aspx
	GOAP programming guides http://alumni.media.mit.edu/~jorkin/goap.html in depth
							https://gamedevelopment.tutsplus.com/tutorials/goal-oriented-action-planning-for-a-smarter-ai--cms-20793 good intro
							http://alumni.media.mit.edu/~jorkin/gdc2006_orkin_jeff_fear.pdf discussion of Goap in fear
							http://alumni.media.mit.edu/~jorkin/GOAP_draft_AIWisdom2_2003.pdf another paper by orkin
	AI resources	https://www.youtube.com/watch?v=GAaBOyMmKok&list=PLokhY9fbx05dShkwWgzweOsSPlgt6OhRM video series
					http://aiandgames.com/ai-101-introduction/ accompanying written material
	goal based agents or utility based agents? https://en.wikipedia.org/wiki/Intelligent_agent
			http://www.gamasutra.com/blogs/JakobRasmussen/20160427/271188/Are_Behavior_Trees_a_Thing_of_the_Past.php in favour of utility - read the commnets, interesting discusion
			http://www.gdcvault.com/play/1021848/Building-a-Better-Centaur-AI
			http://intrinsicalgorithm.com/IAonAI/2013/02/both-my-gdc-lectures-on-utility-theory-free-on-gdc-vault/	giuld wars 2 utility ai gdc lectures
	can i use machine learning to tune the scores of the utility/goal based ai? fitness functions could be difficult
	possibly make the utility/goal ai hierarchical in some way? restrict some actions from being accessed in certain cases to streamline the tree search. parallel ais also suggested
	https://www.reddit.com/r/gameai/comments/25sbk3/what_are_some_good_explanations_of_htn_planners/ interesting goap discussion

	//////////
	or real-game implementation with code examples in C# you can have a look at some of the tutorials we have made:

	http://apexgametools.com/documentation/apex-utility-ai-documentation/?v=dd65ef9a5579

	You can also watch the video tutorials on our Youtube channel:

	https://www.youtube.com/channel/UCXOe80m1hZc2F28QK0Py3cA

	Dave Mark has many useful examples and blog posts on his website:

	http://intrinsicalgorithm.com/

	Guerrilla Games has a very good presentation for procedural tactics:

	https://www.guerrilla-games.com/read/killzones-ai-dynamic-procedural-tactics

	There is also a chapter in Game AI Pro on Utility Theory:

	http://www.aiwisdom.com/

	If you are not familiar with the Game AI Wisdom books they are pretty cool. I always advice anyone building AI to read them A-Z.
	///////////////
*/

#include "stdafx.h"
#include "Game.h"
#include <random>

int main(){
	//int screenX = 1024;
	//int screenY = 768;
	int screenX = 1.5*355;
	int screenY = 1.5*200;
	Game::Start(screenX, screenY);
	/*

	// create source of randomness, and initialize it with non-deterministic seed
	std::random_device r;
	std::seed_seq seed{ r(),r(),r(),r(),r(),r() };
	std::knuth_b eng{ seed };

	// a distribution that takes randomness and produces values in specified range
	std::uniform_int_distribution<> dist(1, 6);

	int cycles = 1000000;

	srand(static_cast <unsigned> (time(0)));
	//static uint32_t xorsfSeed[] = { 123456789, 362436069, 521288629, 88675123 };
	static uint32_t xorsfSeed[] = { rand(), rand(), rand(), rand() };
	float total = 0.0f;
	int count = 0;
	uint32_t max = 0;
	uint32_t min = UINT_MAX;
	clock_t startTime = clock();
	for (int i = 0; i < cycles; i++) {
		uint32_t num = xor128(xorsfSeed);
		//int num = rand();
		if (num > max) { max = num; }
		if (num < min) { min = num; }
		total += (static_cast <float> (num) / static_cast <float> (UINT_MAX));
		count++;
		//std::cout << (static_cast <float> (num) / static_cast <float> (UINT_MAX)) << std::endl;
	}
	std::cout << "xorshift: " << clock() - startTime << std::endl;
	startTime = clock();
	total = 0;
	count = 0;
	for (int i = 0; i < cycles; i++) {
		int num = dist(eng);
		if (num > max) { max = num; }
		if (num < min) { min = num; }
		total += static_cast <float> (num) / static_cast <float> (INT_MAX);
		count++;
	}
	std::cout << "rand(): " << clock() - startTime << std::endl;
	std::cout << total/count << " - " << max << " - " << min << " " << total << " " << RAND_MAX <<  std::endl;*/
	return 0;
}

