
/*
	Code by ����һֻ�����.2015.10.12
	lzw �㷨ʵ��
	compiled success with CL version:12.0
	��windows��ʹ��CL.EXE ����ɹ�
*/
#include <stdio.h>
#include <malloc.h>
#include <windows.h>
#include <string.h>

//===================���ֳ���������============================
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
//===================== �ṹ�� ===============================
	typedef struct
	{
		VOID  *LPIn;      //��ȡλ��ָ�룬������
		VOID  *LPOut;     //д��λ��ָ�룬������
		UBIT32 InSetPos;     // ��ȡ��ʼ��ַ��ͬ��
		UBIT32 OutSetPos;    // д����ʼ��ַ ��������ָ�����������д���ֽ���
		UBIT32 InEndPos;  //��ȡ����λ��ָ�룬������LPIn�Ƚϣ����������ȡ���
		UBIT32 RawSize;   //��ѹ�����ݵ�ԭʼ��С
		UBIT32 TMPBIT32;  //32λ�������ʱ�洢λ�ã�����32λ��д�뻺��
		UBIT32 UsedBits;  //����32λ����ʹ�õ�λ��
	}	LZWIO; 

	typedef struct
	{
		UBIT32 Prefix;  //�ִ�ǰ׺��ǰ��������
		UBIT8  Suffix;  //�ִ���׺
	} TABLE;

	typedef struct
	{
		HANDLE File;
		HANDLE Map;
		VOID *MapMem;
	} MAP;

//=====================ȫ�ֱ���(����)==============================
	LZWIO IO; 				  //IO�ṹ
	TABLE  Stack[STACKMAX];   //��ջ
	UBIT32 StackTop;		  //ջ��λ��
//==============================================================
//IO��غ���
	VOID     LZWIniIO(VOID*,VOID*,UBIT32);
	VOID     IOEndFile();
	VOID     LZWIniZero(VOID*,UBIT32);
    UBIT8    ProcessChain(UBIT32 Index,UBIT8);
	UBIT8    LZWGetByte();
	VOID     LZWWrite(UBIT32);
	UBIT32   LZWPickCode();
//��ջ��غ���
	#define STACKMAX     (4096)
	#define StackIsMax   (256)
	#define StackSet     (258)
	#define NoPrefix     (-1)
	
	
	VOID     LZWIniStack(UBIT32);   
	UBIT32   LZWStackPush(UBIT32,UBIT32);
	UBIT32   LZWStackQuery(UBIT32,UBIT32);
	VOID     PrintStack(UBIT32,UBIT32);
//ѹ����ѹ����
	UBIT32   LZWDeflate(VOID*,VOID*,UBIT32);
	UBIT32   LZWInflate(VOID*,VOID*,UBIT32);
//---------------------------------------------------------------------------------------------------
//���²���Ϊ����Ҫ���룬��Ҫ��Ϊ�˴ճ�һ�������ĳ���
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
	printf("\nLzw ѹ������ By YiDa.Zhang, Achieved With C & LZW Algorithm.(C) Lemple-Ziv-Welch \n");
}
VOID PrintUsage()
{
	printf(">Compress   file:lzw -c [file]\n");
	printf(">Uncompress file:lzw -u [file]\n");
	exit(1);
}
//---------------------------------------------------------------------------------------------------
//��������Ҫʵ�ִ���
UBIT32 LZWInflate(VOID*Src,VOID*Dst,UBIT32 Size)
{  
	UBIT32 Prefix,Suffix;
	UBIT32 Temp;
	LZWIniIO(Src,Dst,Size);
	if(!IsValidFile()) return 0;
	LZWIniStack(True);
//��Ҫ���벿��
	Prefix = LZWPickCode();
	ProcessChain(Prefix,True);
	Suffix = Prefix;
	while(True)
	{
		Temp = LZWPickCode(); 
		if(Temp == TheEnd) break;
		if(Temp == StackIsMax)
		{
			putchar('|');   //��CONSOLE��ӡһ��|ģ�����
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
		{   //�����ڶ�ջ����
			LZWWrite(Prefix);
			//printf("StackPosition=%5d   Code =%6d\n",StackTop,Prefix); //
		    if(LZWStackPush(Prefix,Suffix)==StackIsMax)
		    {   //��ջ������������ˣ�ģ��տ�ʼ��״����
			    //printf("---------------- Table %9d  ---------------\n",COUNT+=1);//
			    putchar('|');
		    	LZWIniStack(False);
		    	LZWWrite(StackIsMax);
		    	//��Ϊ��һ��Suffixû���õ�����Prefix������һ����
		    	Prefix = Suffix;
		    	continue;
		    }
		    Prefix = Suffix;
		    continue;
		}
		Prefix = Index;
	}
	LZWWrite(Prefix);   //�������ʣ�µ�Prefix
	IOEndFile();		//�����ļ�����
	//���ػ��������ֽ���
	return ((UBIT32)IO.LPOut - IO.OutSetPos);
}

VOID IOEndFile()
{   //д�������ǣ����ڲ���12λ����
	//��Ҫ����32λ�Ż�д���ļ�����
	//���������д�˼����������
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
{   //����ѹ�������Ƿ�Ϸ�
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
	//��һ��Prefix��Suffixѹ��ջ��
	if(StackTop!=STACKMAX) 
	{	
		Stack[StackTop].Prefix =  Prefix;
		Stack[StackTop].Suffix =  Suffix;
		return StackTop++;
	}
	return StackIsMax;
}
VOID LZWIniIO(VOID*Src,VOID*Dst,UBIT32 Size)
{	//��ʼ��IO�ṹ
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
	//����ǵڶ��γ�ʼ������ôû��Ҫ�ٴγ�ʼ��ǰ256��
	UBIT32  Index; 
	if(IsFirstIni)
	{  //����һ��������ǰ׺�ĳ�������Ϊ��������г�������0
	   //Ҳ���ǵ�Prefix=Suffix=0������Ϊ���ǲ�����ջ����
	   //�����������Ļ�������᷵����ջ�У�����㶮��
		Index = 0;
		while(Index!=StackSet) LZWStackPush(NoPrefix,Index++);
	}
	//256��257�Ǳ�������������Ҫ����һ�ʼ
	StackTop = StackSet;
}

VOID PrintStack(UBIT32 StartPos,UBIT32 StopPos)
{	//�������һ����ӡ��ջ�ĵ��Ժ�����ɾ��Ҳ�޷�
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
	//��һ��Prefix��Suffixѹ��ջ��
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
	//����Prefix��Suffix�Ƿ���ջ�У�����ڷ�������,���ڷ���False
	UBIT32 Index;
	/*for(Index=StackTop-1;Index!=Prefix;Index--)
	{   //���ѭ������̫���ˣ��ѱ���������Ȼ��𲻴󣬵��ٶȡ�����
		if((Stack[Index].Prefix==Prefix)&&\
		   (Stack[Index].Suffix==Suffix))
		    return Index;
	}*/
	/*
		���ѭ����ܶ࣬����ʱ�����ٶ�ʵ��̫���ˣ�һ��4M���ļ�����10�룩
		�Ͱ�ÿ��������������һ�Σ�����ʱ����Ҫ���������Query���ˣ���ô
		�Ķ�û�ã���������һ�¿�ʼѭ�������Բۣ��������û��Ӧ����!
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
	//��IO.LPInָ���λ�ö�ȡһ���ֽڲ�����ָ��
	return *(((UBIT8*)IO.LPIn)++);
}

UBIT32 LZWGetDword()
{
	//��IO.LPInָ���λ�ö�ȡһ��˫�ֲ�����ָ��
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
{ 	//��������ҵ��Ҷ����뿴�ˣ�ԭ���д������һ����
	//���Ƕ����ĸ��ֽڵı��룬ÿ�δ�����ȡ12λ�����ĸ�
	//�ֽڣ�32λ�����ʣ��λ������ȡʱ���������ĸ��ֽ�
	//�����ĸ���ǰ���ʣ�������Ϸ��ر��롣
	UBIT32 UsedBits  = IO.UsedBits;
	UBIT32 Bit12Code = IO.TMPBIT32;
	UBIT32 Bit32Temp;
	if(UsedBits<12)
	{  //������ʣ���λ������12λ���������ĸ��ֽ�
		Bit32Temp =LZWGetDword();
		if(!UsedBits)
		{ 	//���治������
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


