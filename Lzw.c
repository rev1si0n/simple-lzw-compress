
/*
	Code by 又是一只大大鸟.2015.10.12
	lzw 算法实现
	compiled success with CL version:12.0
	在windows下使用CL.EXE 编译成功
*/
#include <stdio.h>
#include <malloc.h>
#include <windows.h>
#include <string.h>

//===================部分常量及定义============================
	#define UBIT8      unsigned char
	#define UBIT32	   unsigned int
	#define UBIT16	   unsigned short
	#define VOID  	   void
	#define True         (0x1)
	#define False        (0x0)	
	#define STACKMAX     (4096)
	#define StackIsMax   (256)
	#define StackSet     (258)
	#define NoPrefix     (-1)
	//#define NewTab       (256)
	#define TheEnd       (257)
//===================== 结构体 ===============================
	typedef struct
	{
		VOID  *LPIn;      //读取位置指针，会自增
		VOID  *LPOut;     //写入位置指针，会自增
		UBIT32 InSetPos;     // 读取起始地址，同下
		UBIT32 OutSetPos;    // 写入起始地址 ，这两个指针用来计算读写的字节数
		UBIT32 InEndPos;  //读取结束位置指针，用来和LPIn比较，如果相等则读取完毕
		UBIT32 RawSize;   //待压缩数据的原始大小
		UBIT32 TMPBIT32;  //32位编码的临时存储位置，存满32位后将写入缓存
		UBIT32 UsedBits;  //代表32位中已使用的位数
	}	LZWIO; 

	typedef struct
	{
		UBIT32 Prefix;  //字串前缀（前项索引）
		UBIT8  Suffix;  //字串后缀
	} TABLE;

	typedef struct
	{
		HANDLE File;
		HANDLE Map;
		VOID *MapMem;
	} MAP;

//=====================全局变量(必须)==============================
	LZWIO IO; 				  //IO结构
	TABLE  Stack[STACKMAX];   //堆栈
	UBIT32 StackTop;		  //栈顶位置
//==============================================================
//IO相关函数
	VOID     LZWIniIO(VOID*,VOID*,UBIT32);
	VOID     IOEndFile();
	VOID     LZWIniZero(VOID*,UBIT32);
    UBIT8    ProcessChain(UBIT32 Index,UBIT8);
	UBIT8    LZWGetByte();
	VOID     LZWWrite(UBIT32);
	UBIT32   LZWPickCode();
//堆栈相关函数
	#define STACKMAX     (4096)
	#define StackIsMax   (256)
	#define StackSet     (258)
	#define NoPrefix     (-1)
	
	
	VOID     LZWIniStack(UBIT32);   
	UBIT32   LZWStackPush(UBIT32,UBIT32);
	UBIT32   LZWStackQuery(UBIT32,UBIT32);
	VOID     PrintStack(UBIT32,UBIT32);
//压缩解压函数
	UBIT32   LZWDeflate(VOID*,VOID*,UBIT32);
	UBIT32   LZWInflate(VOID*,VOID*,UBIT32);
//---------------------------------------------------------------------------------------------------
//以下部分为非主要代码，主要是为了凑成一个完整的程序
	VOID     PrintUsage();
	VOID     PrintCopyRight();
    UBIT32   CheckCommand(UBIT8*);
    VOID MappingFile(MAP*M,UBIT8*fName,UBIT32 Size,UBIT32 Mod);
    VOID ReleaseMap(MAP*);
    VOID PrintError(char*cname);
int main(int argc,char**argv)
{
	unsigned long FSize,FSizeHigh,WSize,RawSize,choice;
	UBIT8 Fname[1024],*Ext;
	MAP mIn,mOut;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_RED);
	PrintCopyRight();
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_GREEN);

	if(argc==3&&((choice=CheckCommand(argv[1]))!=0))
	{
		if(choice==1)
		{
			strcpy(Fname,argv[2]);
			strcat(Fname,".Ed");
			MappingFile(&mIn,argv[2],0,OPEN_EXISTING);
			if(mIn.File==0) PrintError(argv[2]);
			FSize = GetFileSize(mIn.File,&FSizeHigh);
			MappingFile(&mOut,Fname,FSize*2,CREATE_ALWAYS);
			if(mOut.File==0) PrintError(Fname);
			printf(">Compacting... Please wait.\n");
			WSize = LZWDeflate(mIn.MapMem,mOut.MapMem,FSize);
			ReleaseMap(&mIn);
			ReleaseMap(&mOut);
			SetFilePointer(mOut.File,WSize,0,FILE_BEGIN);
			SetEndOfFile(mOut.File);
			CloseHandle(mOut.File);
			CloseHandle(mIn.File);
			printf("\n>Successful Compact File.\n>>>Raw File Size : %d bytes.\n>>>Compact Size  : %d bytes.\n",FSize,WSize);
			return 0;
		}
		else
		{
			strcpy(Fname,argv[2]);
			if(((Ext=strstr(Fname,".Ed")))==NULL)
			{
				printf("\n>this file's extension name is not '.Ed'\n");
				return 0;
			}
			*Ext = '\0';
			MappingFile(&mIn,argv[2],0,OPEN_EXISTING);
			if(mIn.File==0) PrintError(argv[2]);
			FSize = GetFileSize(mIn.File,&FSizeHigh);
			RawSize = *((UBIT32*)((UBIT8*)mIn.MapMem+FSize-4));
			MappingFile(&mOut,Fname,RawSize+4096,CREATE_ALWAYS);
			if(mOut.File==0) PrintError(Fname);
			printf(">UnCompacting... Please wait.\n");
			if((WSize = LZWInflate(mIn.MapMem,mOut.MapMem,FSize))==0)
			{
				printf(">Not a Valid .Ed File\n");
				exit(1);
			}
			ReleaseMap(&mIn);
			ReleaseMap(&mOut);
			SetFilePointer(mOut.File,WSize,0,FILE_BEGIN);
			SetEndOfFile(mOut.File);
			CloseHandle(mOut.File);
			CloseHandle(mIn.File);
			printf("\n>Successful UnCompact File.\n>>>CheckSum Size : %d bytes.\n>>>UnCompact Size : %d bytes.\n",RawSize,WSize);
			return 0;
		}
	}
	PrintUsage();
}

UBIT32 CheckCommand(UBIT8*Com)
{
	UBIT8 *List[8]={"-c","c","-C","C",\
					"-u","u","-U","U" };
	UBIT32 Index = 0;
	while(Index!=8)
	{
		if(strcmp(Com,List[Index])==0) 
			break;
		Index++;
	}
	return (Index<8)?((Index<4)?1:2):0;
}
VOID PrintError(char*cname)
{
	printf("\n>cannot open(create) file %s\n",cname);
	exit(1);
}
VOID MappingFile(MAP*M,UBIT8*fName,UBIT32 Size,UBIT32 Mod)
{
	M->File = CreateFile(fName,GENERIC_READ+GENERIC_WRITE,\
			FILE_SHARE_READ,0,Mod,FILE_ATTRIBUTE_NORMAL,0);
	if(M->File!=INVALID_HANDLE_VALUE) 
	{
		M->Map =CreateFileMapping(M->File,NULL,PAGE_READWRITE,0,Size,0);
		if(M->Map!=INVALID_HANDLE_VALUE)
		{
			M->MapMem=(void*)MapViewOfFile(M->Map,FILE_MAP_ALL_ACCESS,0,0,0);
		}
	}
	else M->File = 0;
}
VOID ReleaseMap(MAP *M)
{
	UnmapViewOfFile(M->MapMem);
	CloseHandle(M->Map);
	//CloseHandle(M->File);
}

VOID PrintCopyRight()
{
	printf("\nLzw 压缩程序 By YiDa.Zhang, Achieved With C & LZW Algorithm.(C) Lemple-Ziv-Welch \n");
}
VOID PrintUsage()
{
	printf(">Compress   file:lzw -c [file]\n");
	printf(">Uncompress file:lzw -u [file]\n");
	exit(1);
}
//---------------------------------------------------------------------------------------------------
//以下是主要实现代码
UBIT32 LZWInflate(VOID*Src,VOID*Dst,UBIT32 Size)
{  
	UBIT32 Prefix,Suffix;
	UBIT32 Temp;
	LZWIniIO(Src,Dst,Size);
	if(!IsValidFile()) return 0;
	LZWIniStack(True);
//主要代码部分
	Prefix = LZWPickCode();
	ProcessChain(Prefix,True);
	Suffix = Prefix;
	while(True)
	{
		Temp = LZWPickCode(); 
		if(Temp == TheEnd) break;
		if(Temp == StackIsMax)
		{
			putchar('|');   //在CONSOLE打印一个|模拟进度
			LZWIniStack(False);
			Prefix = LZWPickCode();
			ProcessChain(Prefix,True);
			Suffix = Prefix;
			continue;
		}
		Suffix = ProcessChain((Temp<StackTop)?Temp:Prefix,False);
		LZWStackPush(Prefix,Suffix);
		ProcessChain(Temp,True);
		Prefix = Temp;
		//PrintStack(258,266);
	}
	return ((UBIT32)IO.LPOut - IO.OutSetPos);
}

UBIT32 LZWDeflate(VOID*Src,VOID*Dst,UBIT32 Size)
{
	UBIT32 Index; 
	UBIT32 Prefix;
	UBIT8  Suffix;
	//UBIT32 COUNT = 0;  //
	LZWIniStack(True);
	LZWIniIO(Src,Dst,Size);
	Prefix  = LZWGetByte();
	while(IO.LPIn<IO.InEndPos)
	{ 
		Suffix = LZWGetByte();
		if((Index=LZWStackQuery(Prefix,Suffix))==False) 
		{   //这个项不在堆栈中吗？
			LZWWrite(Prefix);
			//printf("StackPosition=%5d   Code =%6d\n",StackTop,Prefix); //
		    if(LZWStackPush(Prefix,Suffix)==StackIsMax)
		    {   //堆栈满了吗？如果满了，模拟刚开始的状况。
			    //printf("---------------- Table %9d  ---------------\n",COUNT+=1);//
			    putchar('|');
		    	LZWIniStack(False);
		    	LZWWrite(StackIsMax);
		    	//因为上一个Suffix没有用掉，而Prefix属于上一个表
		    	Prefix = Suffix;
		    	continue;
		    }
		    Prefix = Suffix;
		    continue;
		}
		Prefix = Index;
	}
	LZWWrite(Prefix);   //处理最后剩下的Prefix
	IOEndFile();		//设置文件结束
	//返回缓冲区的字节数
	return ((UBIT32)IO.LPOut - IO.OutSetPos);
}

VOID IOEndFile()
{   //写入结束标记，由于采用12位缓存
	//需要凑满32位才会写入文件缓存
	//所以这里多写了几个结束标记
	#define EDSign (0x4445)
	#define DTSize (IO.RawSize)
	LZWWrite(TheEnd);
	LZWWrite(TheEnd);
	LZWWrite(TheEnd);
	LZWWrite(TheEnd);
	*(((UBIT16*)IO.LPOut)++) = EDSign;
	*(((UBIT32*)IO.LPOut)++) = DTSize;
}
UBIT32 IsValidFile()
{   //检测解压缩数据是否合法
	// BB BBBB
	return (*(UBIT16*)((UBIT8*)IO.InEndPos-6)\
			== EDSign)?True:False;
}
	#define STACKMAX     (4096)
	#define StackIsMax   (256)
	#define StackSet     (258)
	#define NoPrefix     (-1)
	
	
	VOID     LZWIniStack(UBIT32);   
	UBIT32   LZWStackPush(UBIT32,UBIT32);
	UBIT32   LZWStackQuery(UBIT32,UBIT32);
	VOID     PrintStack(UBIT32,UBIT32);
	
UBIT32 LZWStackPush(UBIT32 Prefix,UBIT32 Suffix)
{
	//将一个Prefix和Suffix压入栈中
	if(StackTop!=STACKMAX) 
	{	
		Stack[StackTop].Prefix =  Prefix;
		Stack[StackTop].Suffix =  Suffix;
		return StackTop++;
	}
	return StackIsMax;
}
VOID LZWIniIO(VOID*Src,VOID*Dst,UBIT32 Size)
{	//初始化IO结构
	IO.LPIn      = Src;
	IO.LPOut     = Dst;
	IO.InSetPos  = (UBIT32)Src;
	IO.OutSetPos = (UBIT32)Dst;
	IO.InEndPos  = ((UBIT32)Src+Size);
	IO.RawSize   = Size;
	IO.TMPBIT32  = 0;
	IO.UsedBits  = 0;
}

VOID LZWIniStack(UBIT32 IsFirstIni)
{
	//如果是第二次初始化表那么没必要再次初始化前256项
	UBIT32  Index; 
	if(IsFirstIni)
	{  //定义一个代表无前缀的常量，因为如果数据中出现两个0
	   //也就是当Prefix=Suffix=0，你认为它是不是在栈中呢
	   //如果不加这个的话，程序会返回在栈中，结果你懂的
		Index = 0;
		while(Index!=StackSet) LZWStackPush(NoPrefix,Index++);
	}
	//256，257是保留数，所以需要从下一项开始
	StackTop = StackSet;
}

VOID PrintStack(UBIT32 StartPos,UBIT32 StopPos)
{	//这仅仅是一个打印堆栈的调试函数，删除也无妨
	HANDLE Console =GetStdHandle(STD_OUTPUT_HANDLE);
	static UBIT32 PrintCount = 0;
	printf("\n--------------------- %6d St Print  ------------------------\n",++PrintCount);
	while(--StopPos!=StartPos) 
	{
		SetConsoleTextAttribute(Console,(StopPos%2==0)?FOREGROUND_RED:FOREGROUND_GREEN);
		printf("Stack[%4d] | Prefix = %5d | Suffix = %3d | StackTop = %6d\n",\
							StopPos,Stack[StopPos].Prefix,Stack[StopPos].Suffix,StackTop);
	}
}


UBIT32 LZWStackPush(UBIT32 Prefix,UBIT32 Suffix)
{
	//将一个Prefix和Suffix压入栈中
	if(StackTop!=STACKMAX) 
	{	
		Stack[StackTop].Prefix =  Prefix;
		Stack[StackTop].Suffix =  Suffix;
		return StackTop++;
	}
	return StackIsMax;
}

UBIT32 LZWStackQuery(UBIT32 Prefix,UBIT8 Suffix)
{  
	//查找Prefix和Suffix是否在栈中，如果在返回索引,不在返回False
	UBIT32 Index;
	/*for(Index=StackTop-1;Index!=Prefix;Index--)
	{   //这个循环方法太慢了，已被抛弃，虽然差别不大，但速度。。。
		if((Stack[Index].Prefix==Prefix)&&\
		   (Stack[Index].Suffix==Suffix))
		    return Index;
	}*/
	/*
		这个循环快很多，运行时发现速度实在太慢了（一个4M的文件用了10秒）
		就把每个函数单独运行一次，发现时间主要都耗在这个Query上了，怎么
		改都没用，后来改了一下开始循环方向，卧槽，快的让我没反应过来!
	*/
	for(Index=Prefix;Index!=StackTop;Index++)
	{	
		if((Stack[Index].Prefix==Prefix)&&\
		   (Stack[Index].Suffix==Suffix))
		   	return Index;		    
	}
	return False;
}

UBIT8 ProcessChain(UBIT32 Index,UBIT8 Mod)
{
	static  UBIT8  Chain[4096];
	UBIT32  CIndex;
	UBIT32  Size;
	CIndex = 4095;
	while(Stack[Index].Prefix!=NoPrefix)
	{
		Chain[CIndex--] = Stack[Index].Suffix;
		Index = Stack[Index].Prefix;
	}
	Chain[CIndex] = Stack[Index].Suffix;
	if(Mod)
	{
		Size = 4096-CIndex;
		memcpy(IO.LPOut,&Chain[CIndex],Size);
		((UBIT32)IO.LPOut)+=Size;
	}
	return Chain[CIndex];
}

VOID LZWIniZero(VOID*dest,UBIT32 nCount)
{  
	__asm
	{
		mov  edi,dest
		mov  eax,0x0
		mov  ecx,nCount
		cld 
		rep stosb
	}
}

UBIT8 LZWGetByte()
{
	//从IO.LPIn指向的位置读取一个字节并递增指针
	return *(((UBIT8*)IO.LPIn)++);
}

UBIT32 LZWGetDword()
{
	//从IO.LPIn指向的位置读取一个双字并递增指针
	return *(((UBIT32*)IO.LPIn)++);
}

VOID LZWWrite(UBIT32 BIT16)
{   
	register UBIT32 BIT32WRITE;
	//printf(">>>Successful Write Code =%6d \n",BIT16);
	if(IO.UsedBits<=20) 
	{ 
		IO.TMPBIT32|=(BIT16<<(32-(IO.UsedBits+=12)));
	}
	else
	{
		if(IO.UsedBits!=32)
		{   
			IO.TMPBIT32|=BIT16>>(IO.UsedBits-20);
			BIT32WRITE  = IO.TMPBIT32;
			IO.TMPBIT32 = BIT16<<(52-IO.UsedBits);
			IO.UsedBits-= 20;
		}
		else
		{
			BIT32WRITE  = IO.TMPBIT32;
			IO.TMPBIT32 = (BIT16<<20);
			IO.UsedBits	= 12;
		}
		*(((UBIT32*)IO.LPOut)++) = BIT32WRITE;
	}
}

UBIT32 LZWPickCode()
{ 	//这个代码乱的我都不想看了，原理和写编码是一样的
	//就是读入四个字节的编码，每次从中提取12位，当四个
	//字节（32位）里的剩余位不够提取时，读入下四个字节
	//把这四个和前面的剩余编码组合返回编码。
	UBIT32 UsedBits  = IO.UsedBits;
	UBIT32 Bit12Code = IO.TMPBIT32;
	UBIT32 Bit32Temp;
	if(UsedBits<12)
	{  //缓存里剩余的位数低于12位，读入下四个字节
		Bit32Temp =LZWGetDword();
		if(!UsedBits)
		{ 	//下面不解释了
			Bit12Code=(Bit32Temp>>20);
			Bit32Temp<<=12;
			UsedBits=20;
			goto OutThis;
		}
		Bit12Code|=(Bit32Temp>>UsedBits);
		Bit32Temp<<=(12-UsedBits);
		Bit12Code>>=(20);
		UsedBits = 20+UsedBits;
	}
	else
	{
		Bit32Temp=Bit12Code<<12;
		Bit12Code>>=(20);
		UsedBits-=12;
	}
	OutThis:
	IO.TMPBIT32  = Bit32Temp;;
	IO.UsedBits  = UsedBits;
	//printf(">>>Successful Get Code %6d \n",Bit12Code);
	return Bit12Code;
}


