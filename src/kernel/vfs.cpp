// 
// Name:	vfs.cpp
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	31-May-2014
// Purpose:	Virtual File System
// 

#include "vfs.h"

static VFS *g_VFS = NULL;


void vfs_Init()
{
	g_VFS = new VFS();

}

//
// RootFS
//
RootFS::RootFS()
{
	m_toplevel = new List<VFSNode*>();
	m_toplevel->Add(new VFSNode("dev", FS_DIRECTORY, 0, 1, this));
	m_toplevel->Add(new VFSNode("drive", FS_DIRECTORY, 0, 2, this));
}

KRESULT RootFS::Read(DWORD inode, DWORD offset, DWORD size, BYTE *buffer, DWORD *bytesread)
{
	return KRESULT_FAILURE;
}

KRESULT RootFS::Write(DWORD inode, DWORD offset, DWORD size, BYTE *buffer, DWORD *byteswritten)
{
	return KRESULT_FAILURE;
}

KRESULT RootFS::Open(DWORD inode, DWORD read, DWORD write)
{
	return KRESULT_FAILURE;
}

KRESULT RootFS::Close(DWORD inode)
{
	return KRESULT_FAILURE;
}

KRESULT RootFS::ReadDir(DWORD inode, DWORD index, VFSNode **node)
{
	List<VFSNode*> *searchlist = NULL;

	switch (inode)
	{
	case 0:
		searchlist = m_toplevel;
		break;
	case 1:
		searchlist = m_devices;
		break;
	default:
		*node = NULL;
		return KRESULT_FAILURE;
	}

	if (index >= searchlist->Count())
	{
		*node = NULL;
		return KRESULT_FAILURE;
	}

	*node = searchlist->Item(index);
	return KRESULT_SUCCESS;
}

KRESULT RootFS::FindDir(DWORD inode, const char *name, VFSNode **node)
{
	List<VFSNode*> *searchlist = NULL;

	switch (inode)
	{
	case 0:
		searchlist = m_toplevel;
		break;
	case 1:
		searchlist = m_devices;
		break;
	default:
		return KRESULT_FAILURE;
	}

	DWORD i = 0;
	for (i = 0; i < searchlist->Count(); i++)
	{
		VFSNode *n = searchlist->Item(i);
		if (n->CompareName(name))
		{
			// Found it
			*node = n;
			return KRESULT_SUCCESS;
		}
	}

	*node = NULL;
	return KRESULT_FAILURE;
}


VFSNode *vfs_GetRootNode()
{
	return g_VFS->GetRootNode();
}

VFS::VFS()
{
	m_rootfs = new RootFS();
	m_rootnode = new VFSNode("/", FS_DIRECTORY, 0, 0, m_rootfs);
}

VFSNode *VFS::Mount(const char *name, FileSystem *fs)
{
	m_rootfs->Mount(name, fs);
}

VFSNode *RootFS::Mount(const char *name, FileSystem *fs)
{
	VFSNode *newnode = new VFSNode(name, FS_DIRECTORY, 0, 0, fs);
	m_devices->Add(newnode);
}

VFSNode::VFSNode(const char *name, DWORD flags, DWORD length, DWORD inode, FileSystem *fs)
{
	strcpy(m_name, name);
	m_flags = flags;
	m_length = length;
	m_inode = inode;
	m_fs = fs;
}

KRESULT VFSNode::Read(DWORD offset, DWORD size, BYTE *buffer, DWORD *bytesread)
{
	return m_fs->Read(m_inode, offset, size, buffer, bytesread);
}

KRESULT VFSNode::Write(DWORD offset, DWORD size, BYTE *buffer, DWORD *byteswritten)
{
	return m_fs->Write(m_inode, offset, size, buffer, byteswritten);
}

KRESULT VFSNode::Open(DWORD read, DWORD write)
{
	return m_fs->Open(m_inode, read, write);
}

KRESULT VFSNode::Close()
{
	return m_fs->Close(m_inode);
}

KRESULT VFSNode::ReadDir(DWORD index, VFSNode **node)
{
	return m_fs->ReadDir(m_inode, index, node);
}

KRESULT VFSNode::FindDir(const char *name, VFSNode **node)
{
	return m_fs->FindDir(m_inode, name, node);
}

