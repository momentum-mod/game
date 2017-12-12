#include "os_utils.h"

#ifdef POSIX

void *GetModuleHandle(const char *name)
{
	void *handle;

	if( name == NULL )
	{
		// hmm, how can this be handled under linux....
		// is it even needed?
		return NULL;
	}

    if( (handle=dlopen(name, RTLD_LAZY))==NULL)
    {
            printf("DLOPEN Error:%s\n",dlerror());
            // couldn't open this file
            return NULL;
    }

	// read "man dlopen" for details
	// in short dlopen() inc a ref count
	// so dec the ref count by performing the close
	dlclose(handle);
	return handle;
}
//returns 0 if successful
int GetModuleInformation_LINUX(const char *name, void **base, size_t *length)
{
	// this is the only way to do this on linux, lol
	FILE *f = fopen("/proc/self/maps", "r");
	if (!f)
		return 1;
	
	char buf[PATH_MAX+100];
	while (!feof(f))
	{
		if (!fgets(buf, sizeof(buf), f))
			break;
		
		char *tmp = strrchr(buf, '\n');
		if (tmp)
			*tmp = '\0';
		
		char *mapname = strchr(buf, '/');
		if (!mapname)
			continue;
		
		char perm[5];
		unsigned long begin, end;
		sscanf(buf, "%lx-%lx %4s", &begin, &end, perm);
		
		if (strcmp(basename(mapname), name) == 0 && perm[0] == 'r' && perm[2] == 'x')
		{
			*base = (void*)begin;
			*length = (size_t)end-begin;
			fclose(f);
			return 0;
		}
	}
	
	fclose(f);
	return 2;
}

#ifdef OSX
//from https://stackoverflow.com/questions/28846503/getting-sizeofimage-and-entrypoint-of-dylib-module
//returns the total size of the dylib binary, including header, data, ...
size_t size_of_image(const mach_header *header)
{
	size_t sz =sizeof(*header); //Size of header
	sz += header->sizeofcmds;	//Size of load commands
	load_command *lc = (load_command *)(header + 1);
	
	for (uint32_t i = 0; i < header->ncmds; i++) {
		if (lc->cmd == LC_SEGMENT)
		{
			sz += ((segment_command*) lc)->vmsize; // Size of segment data
		}
		lc = (load_command*) ((char *) lc + lc->cmdsize);
	}
	return sz;

}
//please kill me
//https://blog.lse.epita.fr/articles/82-playing-with-mach-os-and-dyld.html
int GetModuleInformation_OSX(const char *name, void **base, size_t *length)
{
	task_t task;
	task_dyld_info dlyd_info;
	mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
	
	if (task_info(mach_task_self_, TASK_DYLD_INFO, (task_info_t)&dlyd_info, &count) == KERN_SUCCESS) //request info regarding dylibs in this task (hl2_osx)
	{
		mach_vm_address_t image_infos = dlyd_info.all_image_info_addr;
		dyld_all_image_infos *infos = (dyld_all_image_infos*)image_infos;
		uint32_t image_count = infos->infoArrayCount;
		const dyld_image_info *image_array =  new dyld_image_info[image_count];
		image_array = infos->infoArray;
		
		for(int i = image_count-1; i >= 0; i--) //iterate through all dylibs backwards (since engine.dylib is likely to have been loaded near the end)
		{
			if (V_strstr(image_array[i].imageFilePath, name)) //found it!
			{
				*base = (void*)image_array[i].imageLoadAddress;
				*length = size_of_image(image_array[i].imageLoadAddress);
				printf("Found module %s! Base address: %#08x Length: %lu\n", name, *base, *length);
				delete[] image_array;
				return 0; //success!
			}
		}
		delete[] image_array;
		return 1; //failed to find the given process
	}
	return 2; //failed to find task info
}

#endif //OSX
#endif // POSIX
