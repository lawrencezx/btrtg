#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <elf.h>

#define SECT_SIZE 512

const char *bootloc = "./bootloader";
const char *srvloc = "./service";
const char *imgloc = "./image";

const int offs_srvsize = 2;
const int offs_srvoffs = 6;

int main(){
    printf("\nCreating VM disk image\n\n");

    struct stat stat_boot;
    struct stat stat_srv;

    //Verifying boot loader
    if(stat(bootloc,&stat_boot) == -1){
        printf("Could not open %s - exiting\n", bootloc);
        return errno;
    }

    if(stat_boot.st_size > SECT_SIZE){
        printf("Boot sector too big! (%ld bytes)\n", stat_boot.st_size);
        return 99;
    }

    int boot_sect = stat_boot.st_size / SECT_SIZE + 
    (stat_boot.st_size % SECT_SIZE > 0 ? 1 : 0);  

    printf("Size of bootloader:\t %ld bytes, => %d sectors.\n", 
        stat_boot.st_size, boot_sect);
  
     //Verifying service 
    if(stat(srvloc, &stat_srv)==-1){
        printf("Could not open %s - exiting.\n", srvloc);
        return errno;
    }

    int srv_sect=stat_srv.st_size / SECT_SIZE + 
        (stat_srv.st_size % SECT_SIZE > 0 ? 1 : 0);
    printf("Size of service: \t%ld bytes, => %d sectors.\n", stat_srv.st_size,
            srv_sect);
  
    printf("Total disk size: \t%d bytes, => %d sectors.\n",
            (boot_sect + srv_sect) * SECT_SIZE, boot_sect + srv_sect);

    //int disksize=boot_sect*SECT_SIZE + srv_sect*SECT_SIZE;
    /* 
        Bochs requires old-school disk specifications. 
        sectors=cyls*heads*spt (sectors per track)
    */
    int cylinders=306, heads=4, spt=17;
    int disksize=cylinders*heads*spt*SECT_SIZE;
    printf("Creating disk of size: \nCyls: %d\nHeads: %d\nSec/Tr: %d\n=> %d sectors\n=> %d bytes\n",
            cylinders, heads, spt, disksize/SECT_SIZE, disksize);
  
    char *disk = (char *)malloc(disksize * sizeof(char));

    //Load the boot loader into memory
    FILE *file_boot = fopen(bootloc,"r");  
    printf("Read %ld bytes from boot image\n",
            fread(disk, 1, stat_boot.st_size, file_boot));

    //Load the service into memory
    FILE* file_srv=fopen(srvloc,"r");
  
     //Location of service code within the image
    char* srv_imgloc=disk+(boot_sect*SECT_SIZE);  
    printf("Read %ld sectors from service image\n",
            fread(srv_imgloc, 1, stat_srv.st_size, file_srv));
  
    /* 
        ELF Header summary
    */
    Elf32_Ehdr* elf_header=(Elf32_Ehdr*)srv_imgloc;
    printf("Reading ELF headers...\n");
    printf("Signature: ");
    for(int i=0;i<EI_NIDENT;i++)
        printf("%c", elf_header->e_ident[i]);  
    printf("\n");
    printf("Type: %s\n", (elf_header->e_type==ET_EXEC ? " ELF Executable " : "Non-executable"));
    printf("Machine: ");
    switch(elf_header->e_machine){
    case(EM_386):
        printf("Intel 80386");
        break;
    case(EM_X86_64) :
        printf("Intel x86_64");
        break;
    default:
        printf("UNKNOWN (%d)", elf_header->e_machine);
        break;
    }
    printf("\n");
    printf("Version: %d\n", elf_header->e_version);
    printf("Entry point: 0x%x\n", elf_header->e_entry);
    printf("Number of program headers: %d\n", elf_header->e_phnum);
    printf("Program header offset: %d\n", elf_header->e_phoff);
    printf("Number of section headers: %d\n", elf_header->e_shnum);
    printf("Section hader offset: %d\n", elf_header->e_shoff);
    printf("Size of ELF-header: %d bytes\n", elf_header->e_ehsize);

    // END Elf-header summary
  
    printf("\nFetching offset of section .text (the service starting point)\n");
  
    Elf32_Phdr* prog_hdr=(Elf32_Phdr*)(srv_imgloc+elf_header->e_phoff);
    printf("Pheader 1, location: %d\n", prog_hdr->p_offset);
    int srv_start=prog_hdr->p_offset;
  
    //Write OS/Service size to the bootloader
    *((int*)(disk+offs_srvsize))=srv_sect;
    *((int*)(disk+offs_srvoffs))=srv_start;

    //Write the image
    FILE* image=fopen(imgloc,"w");
    int wrote=fwrite((void*)disk,1,disksize,image);
    printf("Wrote %d bytes => %d sectors to %s\n", wrote, wrote / SECT_SIZE, imgloc);

    //Cleanup
    fclose(file_boot);
    fclose(file_srv);  
    fclose(image);

    free(disk);
}
