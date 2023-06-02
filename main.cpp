#include "Fixed_Length_Memory_Pool.hpp"
//#include "Automatic_Expand_Fixed_Length_Memory_Pool.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <time.h>

//#define printf
//0x7fffffff//数组上限
//0x80000000//映像上限

#define BLOCK_SIZE 8
//#define BLOCK_NUM 134217728
#define BLOCK_NUM (67108864)
//#define BLOCK_NUM 32768

#define CLOCKTOSEC(start,end) ((long double)((end) - (start)) / (long double)CLOCKS_PER_SEC)


void *pArr[BLOCK_NUM];

//性能测试
int main(void)
{
	printf("info:\n    block size=%ldByte\n     block num=%ld\n      mem size=%.4lfMB\n", BLOCK_SIZE, BLOCK_NUM, (long double)BLOCK_SIZE * (long double)BLOCK_NUM / (long double)1024 / (long double)1024);
	clock_t start, alloc, end;

	start = clock();
	{
		FixLen_MemPool<void, false> a(BLOCK_SIZE, BLOCK_NUM);
		for (int i = 0; i < BLOCK_NUM; ++i)
		{
			pArr[i] = a.AllocMemBlock();
		}
		alloc = clock();
		for (int i = 0; i < BLOCK_NUM; ++i)
		{
			a.FreeMemBlock(pArr[i]);
		}
	}
	end = clock();
	clock_t my = end - start;
	printf("my:\n    alloc=%.4lfs\n     free=%.4lfs\n      all=%.4lfs\n\n", CLOCKTOSEC(start, alloc), CLOCKTOSEC(alloc, end), CLOCKTOSEC(start, end));


	start = clock();
	{
		for (int i = 0; i < BLOCK_NUM; ++i)
		{
			pArr[i] = new char[BLOCK_SIZE];
		}
		alloc = clock();
		for (int i = 0; i < BLOCK_NUM; ++i)
		{
			delete[] (char *)pArr[i];
		}
	}
	end = clock();
	clock_t n = end - start;
	printf("new:\n    alloc=%.4lfs\n     free=%.4lfs\n      all=%.4lfs\n\n", CLOCKTOSEC(start, alloc), CLOCKTOSEC(alloc, end), CLOCKTOSEC(start, end));
	

	printf("ratio:\n");
	printf("    new/my=%.6lf\n\n", (long double)n / (long double)my);
	return 0;
}


#include <iostream>
#include <chrono>
using namespace std;

struct Ctest
{
	int a;
	int b;
	int c;
};

#define SIZE sizeof(Ctest)
#define MAX_CYCLE 134217728/256

void *arr[MAX_CYCLE];

int mainl(void)
{
	using Micro = std::chrono::duration<double, micro>;
#define Now() chrono::system_clock::now();

	auto start = Now();
	auto end = Now();

	start = Now();
	{
		FixLen_MemPool<void, false> mem_pool(SIZE, MAX_CYCLE);
		for (int i = 0; i < MAX_CYCLE; ++i)
			arr[i] = mem_pool.AllocMemBlock();
		for (int i = 0; i < MAX_CYCLE; ++i)
			mem_pool.FreeMemBlock(arr[i]);
	}
	end = Now();
	std::cout << Micro(end - start) << std::endl;

	start = Now();
	{
		for (int i = 0; i < MAX_CYCLE; i++)
			arr[i] = new char[SIZE];
		for (int i = 0; i < MAX_CYCLE; ++i)
			delete[](char *)arr[i];
	}
	end = Now();
	std::cout << Micro(end - start) << std::endl;

	start = Now();
	{
		for (int i = 0; i < MAX_CYCLE; i++)
			arr[i] = new Ctest;
		for (int i = 0; i < MAX_CYCLE; ++i)
			delete arr[i];
	}
	end = Now();
	std::cout << Micro(end - start) << std::endl;

	return 0;
}

#include <limits.h>

//随机分配释放测试
int maing(void)
{
	//FixLen_MemPool<void> a(10, 1024);
	FixLen_MemPool<void, false> a(10, 1024);
	std::vector<void *> v;

	//分配一部分
	for (int i = 0; i < 1024 / 2; ++i)
	{
		v.push_back(a.AllocMemBlock());
		//printf("%d alloc:0x%p\n", i, v.back());
	}
	a.Reset();//全部释放

	//随机释放+分配
	srand(2);
	void *pTemp;

	for (int i = 0; i < INT_MAX; ++i)
	{
		if (rand() % 2 == 0)//分配
		{
			pTemp = a.AllocMemBlock();
			if (pTemp != nullptr)
			{
				v.push_back(pTemp);
				printf("alloc:0x%p\t", pTemp);
			}
			else
			{
				printf("alloc:fail\t\t\t");
			}
		}
		else//释放
		{
			if (v.size() == 0)
			{
				continue;
			}
			//随机一个幸运元素
			int iTemp = rand() % v.size();

			pTemp = v[iTemp];
			std::swap(v[iTemp], v.back());//与队末元素交换
			v.pop_back();//弹出（删除）

			if (a.FreeMemBlock(pTemp) == true)//释放
			{
				printf("free :0x%p\t", pTemp);
			}
			else
			{
				printf("free :fail\t\t\t");
			}
		}

		if (i % 5 == 0)
		{
			printf("\n");
		}
	}

	return 0;
}


#define MY
using my = FixLen_MemPool<void, false>;

#ifdef MY
#define INIT(s,n)	my a(s, n)
#define ALLOC(s)	a.AllocMemBlock()
#define FREE(p)		a.FreeMemBlock(p)
#define UNIN()		a.~my();
/*
time:
	 init :0.1070s
	 alloc:0.4440s
	 freem:0.5630s
	 unin :0.0250s
	 all  :1.1390s
*/
#elif defined NEW
#define INIT(s,n)	//do nothing
#define ALLOC(s)	new char[s]
#define FREE(p)		delete[](char*)p
#define UNIN()		//do nothing
/*
time:
	 init :0.0000s
	 alloc:2.3530s
	 freem:2.5900s
	 unin :0.0000s
	 all  :4.9430s
*/
#elif defined MALLOC//
#define INIT(s,n)	//do nothing
#define ALLOC(s)	malloc(s)
#define FREE(p)		free(p)
#define UNIN()		//do nothing
/*
time:
	 init :0.0000s
	 alloc:2.6430s
	 freem:3.2420s
	 unin :0.0000s
	 all  :5.8850s
*/
#endif // MY

#undef CLOCKTOSEC
#define CLOCKTOSEC(val) (((long double)(val)) / (long double)CLOCKS_PER_SEC)

//性能测试
int mainj(void)
{
	clock_t init, alloc, freem, unin, all;

	{
		init = clock();
		INIT(BLOCK_SIZE, BLOCK_NUM);
		init = clock() - init;

		printf("init:ok\n\n");

		//先全部请求完
		//一共执行16次
		for (int j = 0; j < 16; ++j)
		{
			printf("for:%d\n", j);
			//初始化随机种子（生成固定伪随机数序列）
			srand(j);
			printf("  srand:ok\n");

			alloc = clock();
			for (int i = 0; i < BLOCK_NUM; ++i)
			{
				pArr[i] = ALLOC(BLOCK_SIZE);
			}
			alloc = clock() - alloc;

			printf("  alloc:ok\n");

			for (int i = BLOCK_NUM - 1; i >= 0; --i)
			{
				int f = rand() % (i + 1);//挑选一个随机幸运元素释放
				//挑选到的元素与末尾交换
				std::swap(pArr[f], pArr[i]);
			}

			printf("  rand:ok\n");

			freem = clock();
			for (int i = 0; i < BLOCK_NUM; ++i)
			{
				//释放末尾元素
				FREE(pArr[i]);
			}
			freem = clock() - freem;

			printf("  free:ok\n");
		}

		unin = clock();
		UNIN();
		unin = clock() - unin;

		printf("unin:ok\n\n");
	}
	
#ifdef MY
	all = init + alloc + freem + unin;
#else
	all = alloc + freem;
#endif // MY


	printf(
		"time:\n"\
		"     init :%.4lfs\n"\
		"     alloc:%.4lfs\n"\
		"     freem:%.4lfs\n"\
		"     unin :%.4lfs\n"\
		"     all  :%.4lfs\n",
		CLOCKTOSEC(init),
		CLOCKTOSEC(alloc),
		CLOCKTOSEC(freem),
		CLOCKTOSEC(unin),
		CLOCKTOSEC(all)
	);

	return 0;
}