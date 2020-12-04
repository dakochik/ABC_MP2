#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <random>

const std::string SIZE_TYPE = "UK";

std::mutex mtx; // Global mutex for every thread;

// We have the following construction (3 mutexes) because we need 3 different queues and because std::mutex doesn't have copy constructor.
std::mutex mtx1; // Mutex for reseller #1
std::mutex mtx2; // Mutex for reseller #2
std::mutex mtx3; // Mutex for reseller #3

std::minstd_rand gen;
std::uniform_real_distribution<double> dist; // We use special library for normal (correct) random generation with multitasking.

class Reseller
{
public:

	Reseller(std::string* items, int arrSize)
	{
		int itemsNumber = MIN_ITEMS_NUMB + (((int)dist(gen)) % (MAX_ITEMS_NUMB - MIN_ITEMS_NUMB + 1));

		for (; itemsNumber > 0; --itemsNumber)
		{
			double size = (((int)dist(gen)) % 14) + 1.0;
			size += (((int)dist(gen)) % 2) == 0 ? 0.5 : 0;

			int pos = ((int)dist(gen)) % arrSize;

			warehouse.emplace_back(std::pair<std::string, double>(items[pos], size));
		}
		std::cout << "Reseller #" << totalNumb++ << " is ready to sel all " << warehouse.size() << " sneakers" << std::endl;
	}

	std::pair<std::string, double> sellItem()
	{
		int pos = rand() % warehouse.size();
		auto result = warehouse[pos];
		warehouse.erase(warehouse.begin() + pos);
		return result;
	}

	bool isHeEmpty() { return  warehouse.empty(); }

	std::string getInfo()
	{
		if (warehouse.empty())
		{
			return "I was successful and sold all sneakers!";
		}
		return "It wasn't well enough, I've " + std::to_string(warehouse.size()) + " pairs left.";
	}

	static int MAX_ITEMS_NUMB;
	static int MIN_ITEMS_NUMB;

private:
	std::vector<std::pair<std::string, double>> warehouse;
	static int totalNumb;
};

int Reseller::totalNumb = 1;

std::vector<Reseller> resellers;

void consumerDoShopping(int consNumber)
{
	bool stopShopping = false;

	do
	{
		
		std::unique_lock<std::mutex> lock(mtx);
		
		int resellerNumber = ((int)dist(gen)) % 3;
		stopShopping = ((int)dist(gen)) % 2;
		
		std::unique_lock<std::mutex>* lockR;
		lock.unlock();
		
		switch (resellerNumber)
		{
		case(0):
			lockR = new std::unique_lock<std::mutex>(mtx1);
			break;
		case(1):
			lockR = new std::unique_lock<std::mutex>(mtx2);
			break;
		default:
			lockR = new std::unique_lock<std::mutex>(mtx3);
			break;
		}

		lock.lock();
		std::cout<< "[START] Consumer #" << consNumber + 1 << "   started deal with   " << "Reseller #" << resellerNumber + 1 <<"\n"<< std::endl;
		lock.unlock();
		
		if (!resellers[resellerNumber].isHeEmpty())
		{
			auto item = resellers[resellerNumber].sellItem();

			lock.lock();
			std::cout << "[FINISH] Consumer #" << consNumber + 1 << "   finished deal with   " << "Reseller #" << resellerNumber + 1<< std::endl;
			std::cout << "\\\\======>" << item.first + " " + SIZE_TYPE + " " << item.second << "\n" << std::endl;
			lock.unlock();
		}
		else
		{
			lock.lock();
			std::cout << "Consumer #" << consNumber + 1 << ": nothing interesting here...\n" << "(Reseller #"<< resellerNumber + 1 <<" warehouse)\n"<<std::endl;
			lock.unlock();
		}
		lockR->unlock();

	} while (!stopShopping);
}

void printInfo()
{
	std::cout << "Incorrect arguments please, try One of the following options:" << std::endl;
	std::cout << "1) <max consumers number> <min items number> <max items number> - 3 positive integer arguments" << std::endl;
	std::cout << "2) <min items number> <max items number> - 2 positive integer arguments" << std::endl;
	std::cout << "3) <max consumers number> - 1 positive integer arguments" << std::endl;
	std::cout << "4)  - 0 arguments. Program uses default values." << std::endl;
}

int Reseller::MIN_ITEMS_NUMB = 0;
int Reseller::MAX_ITEMS_NUMB = 0;

int getArguments(std::string a1, std::string a2, std::string a3)
{
	try
	{
		int consNum = std::stoi(a1);
		Reseller::MIN_ITEMS_NUMB = std::stoi(a2);
		Reseller::MAX_ITEMS_NUMB = std::stoi(a3);

		if (consNum <= 0 || Reseller::MIN_ITEMS_NUMB < 0 || Reseller::MIN_ITEMS_NUMB >= Reseller::MAX_ITEMS_NUMB)
		{
			printInfo();
			return -1;
		}
		return consNum;
	}
	catch (...)
	{
		printInfo();
		return -1;
	}
}

int main(int argc, char* argv[])
{
	int consumersNumb;

	switch (argc)
	{
	case 1:
		consumersNumb = getArguments("4", "10", "30");
		break;
	case 2:
		consumersNumb = getArguments(argv[1], "10", "30");
		break;
	case 3:
		consumersNumb = getArguments("4", argv[1], argv[2]);
		break;
	case 4:
		consumersNumb = getArguments(argv[1], argv[2], argv[3]);
		break;
	default:
		printInfo();
		return 0;
	}

	if(consumersNumb == -1)
	{
		return 0;
	}
	
	gen = std::minstd_rand(std::random_device{}());
	dist = std::uniform_real_distribution<double>(0, 100);

	std::string models[12] = { "Air Jordan 1 Mid Chicago Toe", "Air Jordan 1 Retro High Obsidian UNC", "Air Jordan 11 Retro Concord",
	"Air Jordan 1 Retro High Travis Scott", "Air Jordan 1 Retro High Bred Toe", "Adidas YEEZY 350 V2 Black Red", "Adidas YEEZY 350 V2 Zebra",
	"Adidas YEEZY 500 Stone", "Adidas YEEZY 500 Soft Vision", "Adidas YEEZY 700 V2 Hospital Blue", "Adidas YEEZY 700 V2 Tephra", "Adidas YEEZY 700 V3 Safflower" };

	std::cout << "These are our sellers:" << std::endl;
	resellers.emplace_back(Reseller(models, 12));
	resellers.emplace_back(Reseller(models, 12));
	resellers.emplace_back(Reseller(models, 12));

	std::cout << "\n~ ~ ~ ~ ~ ~ ~ ~ Shopping mall is opened!!! ~ ~ ~ ~ ~ ~ ~ ~\n" << std::endl;

	std::vector<std::thread> consumers;
	for (int i = 0; i < consumersNumb; ++i)
	{
		std::thread thr(consumerDoShopping, i);
		consumers.emplace_back(std::move(thr));
	}
	for (auto& thr : consumers) {
		thr.join();
	}

	std::cout << "It's 22:00, shopping mall is closed.\nThere some results:" << std::endl;
	std::cout << "Reseller #" << 1 << ": " << resellers[0].getInfo() << std::endl;
	std::cout << "Reseller #" << 2 << ": " << resellers[1].getInfo() << std::endl;
	std::cout << "Reseller #" << 3 << ": " << resellers[2].getInfo() << std::endl;
}