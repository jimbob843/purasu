// 
// Name:	vfs.h
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	31-May-2014
// Purpose:	Virtual File System
// 

#ifndef __VFS_H__
#define __VFS_H__

#include "kernel.h"
#include "list.h"

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08 // Is the file an active mountpoint?


typedef struct direntry
{
	char name[256];
	DWORD inode;
} direntry_t;

class VFSNode;

//typedef KRESULT (*read_type_t)(DWORD,DWORD,BYTE*,DWORD*);
//typedef KRESULT (*write_type_t)(DWORD,DWORD,BYTE*,DWORD*);
//typedef KRESULT (*open_type_t)(DWORD read, DWORD write);
//typedef KRESULT (*close_type_t)();
//typedef KRESULT (*readdir_type_t)(DWORD, VFSNode**);
//typedef KRESULT (*finddir_type_t)(char *name, VFSNode**); 


class FileSystem
{
public:
	virtual KRESULT Read(DWORD inode, DWORD offset, DWORD size, BYTE *buffer, DWORD *bytesread) = 0;
	virtual KRESULT Write(DWORD inode, DWORD offset, DWORD size, BYTE *buffer, DWORD *byteswritten) = 0;
	virtual KRESULT Open(DWORD inode, DWORD read, DWORD write) = 0;
	virtual KRESULT Close(DWORD inode)= 0;
	virtual KRESULT ReadDir(DWORD inode, DWORD index, VFSNode **node) = 0;
	virtual KRESULT FindDir(DWORD inode, const char *name, VFSNode **node) = 0;

	virtual VFSNode *Mount(const char *name, FileSystem *fs) = 0;
};

class VFSNode
{
	char m_name[256];
	DWORD m_flags;
	DWORD m_length;
	DWORD m_inode;
	FileSystem *m_fs;

public:
	VFSNode(const char *name, DWORD flags, DWORD length, DWORD inode, FileSystem *fs);

	KRESULT Read(DWORD offset, DWORD size, BYTE *buffer, DWORD *bytesread);
	KRESULT Write(DWORD offset, DWORD size, BYTE *buffer, DWORD *byteswritten);
	KRESULT Open(DWORD read, DWORD write);
	KRESULT Close();
	KRESULT ReadDir(DWORD index, VFSNode **node);
	KRESULT FindDir(const char *name, VFSNode **node);

	bool CompareName(const char *othername)
	{
		return (strcmp(m_name, othername) == 0);
	}

	const char *Name()
	{
		return m_name;
	}
};

class RootFS : public FileSystem
{
	List<VFSNode*> *m_toplevel;		// inode = 0
	List<VFSNode*> *m_devices;		// inode = 1

public:
	RootFS();

	virtual KRESULT Read(DWORD inode, DWORD offset, DWORD size, BYTE *buffer, DWORD *bytesread);
	virtual KRESULT Write(DWORD inode, DWORD offset, DWORD size, BYTE *buffer, DWORD *byteswritten);
	virtual KRESULT Open(DWORD inode, DWORD read, DWORD write);
	virtual KRESULT Close(DWORD inode);
	virtual KRESULT ReadDir(DWORD inode, DWORD index, VFSNode **node);
	virtual KRESULT FindDir(DWORD inode, const char *name, VFSNode **node);

	virtual VFSNode *Mount(const char *name, FileSystem *fs);
};

class VFS
{
	VFSNode *m_rootnode;
	FileSystem *m_rootfs;

public:
	VFS();

	VFSNode *GetRootNode()
	{
		return m_rootnode;
	}

	VFSNode *Mount(const char *name, FileSystem *fs);
};


extern void vfs_Init();
extern VFSNode *vfs_GetRootNode();


#endif // __VFS_H__
