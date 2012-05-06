#include<sys/ioctl.h>
#include<fcntl.h>
#include<termios.h>
#include<sys/types.h>
#include<stdio.h>
#include<errno.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>

#include"/sys/dev/ida/idareg.h"
#include"/sys/dev/ida/idaio.h"

void 
usage() {
	printf("IDA Controller: monitor the IDA drives status.\n");
	printf("Usage:\n");
	printf("  ida_controller [-h] -d disk_no -f device\n");
	printf("  -h		print this help\n");
	printf("  -d disk_no	disk to probe\n");
	printf("  -f device	device to probe\n");
	exit(1);
}

void 
print_result(int cmd,int error,struct ida_user_command iuc)
{/*Analyse and print almost all the data obtained*/
	if(error==0)
	{
		/*printf("Drive: %d\n",iuc.drive);*/

		switch(cmd) {
		case CMD_SENSE_DRV_STATUS:;
			struct ida_drive_status ids = (iuc.d).ds;
			printf("Status: %d\n",(int)ids.status);
			printf("Failure_map: %d\n",(int)ids.failure_map);
			printf("Rebuilding: %d\n",(int)ids.rebuilding);
			printf("Media exchange: %d\n",(int)ids.media_exchange);
			printf("Cache failure: %d\n",(int)ids.cache_failure);
			printf("Expand failure: %d\n",(int)ids.expand_failure);
			printf("Unit flags: %d\n",(int)ids.unit_flags);
			break;
		case CMD_GET_LOG_DRV_EXT:;
			struct ida_drive_info_ext idie = (iuc.d).die;
			printf("Block size [bytes]: %d\n",(int)idie.secsize);
			printf("Blocks available: %d\n",(int)idie.secperunit);
			printf("Mirror(fault tollerance): %d\n",(int)idie.mirror);
			printf("Logical drive identifier: %d\n",idie.ld_id);
			printf("Logical drive label: %s\n",(char*)idie.ld_label);
			break;
		case CMD_GET_CTRL_INFO:;
			struct ida_controller_info ici = (iuc.d).ci;
			printf("Number of configured logical drives: %d\n",(int)ici.num_drvs);
			printf("Configuration signature: %d\n",(int)ici.signature);
			printf("Firmware revision: %s\n",(char*)ici.firm_rev);
			printf("ROM firmware revision: %s\n",(char*)ici.rom_rev);
			break;
		case CMD_GET_PHYS_DRV_INFO:;
			struct ida_phys_drv_info ipdi = (iuc.d).pdi;
			printf("SCSI Bus: %d\n",(int)ipdi.scsi_bus);
			printf("SCSI ID: %d\n",(int)ipdi.scsi_id);
			printf("Block size [bytes]: %d\n",(int)ipdi.blksize);
			printf("Total blocks: %d\n",(int)ipdi.blkcount);
			printf("Reserved blocks: %d\n",(int)ipdi.blkreserved);
			printf("Drive model: %s\n",(char*)ipdi.drv_model);
			printf("Drive serial: %s\n",(char*)ipdi.drv_serial);
			printf("Drive firmware: %s\n",(char*)ipdi.drv_fwrev);
			break;
		case CMD_LABEL_LOG_DRV:;
			struct ida_label_logical ill = (iuc.d).ll;
			printf("Logical drive identifier: %d\n",(int)ill.ld_id);
			printf("Logical drive label: %s\n",(char*)ill.ld_label);
			break;
		default :
			printf("Unrecognized command.\n");
		}

	}
	else switch(error)
	{
		case EINVAL:
			printf("Invalid user command.\n");
			break;
		default :
			printf("Something went wrong...\nError: %d\n",error);

	}

}

void
parse_parms(int argc,char** argv,int* disk,char** filename)
{
	int i;
	while ((i=getopt(argc,argv,"hd:f:"))!=-1) {
		switch(i) {
		case 'h':
			usage();
			exit(-1);
			break;
		case 'f':
			(*filename) = (char*)optarg;
			break;
		case 'd':
			(*disk) = strtol(optarg,NULL,10);
			break;
		case '?':
			printf("Unrecognized option: %c\n",optopt);
		default :
			usage();
		}
	}
}
void
prob_data(int fd,struct ida_user_command* iuc) {
/* This is not used but useful for probing*/
	int cmds[] = {CMD_SENSE_DRV_STATUS,CMD_GET_LOG_DRV_EXT,CMD_GET_CTRL_INFO,CMD_GET_PHYS_DRV_INFO,CMD_LABEL_LOG_DRV};
	char* cmd_desc[] = {"Sense drive status","Log drive extended","Control information","Physical drive information","Label log drive"};
	int cmds_len = 5;
	int i;
	for(i=0;i<cmds_len;i++){
		iuc->command = cmds[i];	
		printf("***********************\n");
		printf("Now showing: %s\n",cmd_desc[i]);
		int error = ioctl(fd,IDAIO_COMMAND,(char*)(iuc),0,0);
		print_result(cmds[i],error,*iuc);
	}
}
void
print_status(int fd,struct ida_user_command* iuc){
	iuc->command = CMD_SENSE_DRV_STATUS;
	int error = ioctl(fd,IDAIO_COMMAND,(char*)(iuc),0,0);
	if(error==0){
		struct ida_drive_status ids = (iuc->d).ds;
		int status = (int)ids.status;
		switch (status) {
		case 0:
			printf("Ehi, everything is OK!\n");
			break;
		case 2:
			printf("This logical drive is not active.\n");
			printf("(Maybe you have digited a wrong parameter for the -d option.\n)");
			break;
		case 3:
			printf("One physical drive is not active!!\n");
			printf("Substitute it NOW!\n");
			break;
		case 4:
			printf("There is an incosistence among the physical drives.\n");
			break;
		case 5:
			printf("The physical drives are synchronizing...\n");
			break;
		default :
			printf("A state not known has been detected:\n\tState = %d\n",status);
		}
	}else
		printf("Something went wrong...\nError: %d\n",error);

}
int 
main(int argc,char** argv)
{
	char* device = "";
	struct ida_user_command iuc;
	iuc.blkno=0;
	iuc.drive = -1;
	parse_parms(argc,argv,&(iuc.drive),&device);

	if(iuc.drive>=0 && strcmp(device,"")!=0) {
		int fd = open(device,O_RDONLY);
		
		print_status(fd,&iuc);	
	
		return 0;
	}
	else
		usage();
	return -1;
}
