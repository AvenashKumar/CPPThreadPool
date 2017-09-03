#include <iostream>
#include <string>
#include "cThreadPool.h"
using namespace std;
void job(const int p_ciJobNumber)
{
	cout << "Job " + to_string(p_ciJobNumber) + " Started" << endl;
	//TODO: No mutual exculsive code
	cout << "Job " + to_string(p_ciJobNumber) + " Done" << endl;
}

void main()
{
	{
		//Total worker threads
		const int iTotalWorkers = 4;
		cThreadPool threadPool(iTotalWorkers);

		//Total jobs
		const int iTotalJobs = 10;
		for (int iJob = 0; iJob < 10; ++iJob)
		{
			//Bind job parameters
			auto aJob = bind(job, iJob);
			//Assign job
			threadPool.enqueue(aJob);
		}
	}

}