// Declare what kind of code we want
// from the header files. Defining __KERNEL__
// and MODULE allows us to access kernel-level
// code not usually available to userspace programs.
#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE
//#include <errno.h>
#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/slab.h>
#include <linux/uaccess.h>  /* for get_user and put_user */
#include <linux/string.h>   /* for memset. NOTE - not string.h!*/
MODULE_LICENSE("GPL");

#include "message_slot.h"
static channelList massageSlotsDeviceFilesList[257]; //256 + one for the end of the array sing 

//================== DEVICE FUNCTIONS ===========================
static int device_open( struct inode *inode, struct file *file )
{
  return SUCCESS;
}

//----------------------------------------------------------------
static long device_ioctl( struct file* file, unsigned int ioctl_command_id, unsigned long ioctl_param ){
  int curChannelMinorNum; 
  channel *channelPointer;
  channel *tmp;
  int i;
  printk("start");
  //If the passed command is not MSG_SLOT_CHANNEL, the ioctl() returns -1 and errno is set to EINVAL.
  if(ioctl_command_id != MSG_SLOT_CHANNEL){
    return -EINVAL;
  }
  //If the passed channel id is 0, the ioctl() returns -1 and errno is set to EINVAL.
  if(ioctl_param == 0){
    return -EINVAL;
  }
  curChannelMinorNum = iminor(file->f_inode);
  channelPointer = massageSlotsDeviceFilesList[curChannelMinorNum].first;
  i = 0;
  while (i == 0){
    if (channelPointer == NULL){
      i = 1;
    }
    else {
      if(channelPointer->minorNumber == ioctl_param){
        i = 2;
      }
      else {
        channelPointer = channelPointer->next;
      }
    }
  }
  // there is no channel with the specified minorNumber
  if(i == 1){
    printk("middle");
    channelPointer = (channel *)kmalloc(sizeof(channel), GFP_KERNEL);
    // failing to allocate memory
    if (channelPointer == NULL) {
      return -EINVAL;
    }
    channelPointer->minorNumber = ioctl_param;
    channelPointer->mesLen = 0;
    channelPointer->next = NULL;
  }
  tmp = massageSlotsDeviceFilesList[curChannelMinorNum].first;
  massageSlotsDeviceFilesList[curChannelMinorNum].first = channelPointer;
  channelPointer->next = tmp;
  file->private_data = channelPointer;
  printk("end");
  return SUCCESS;
}

//---------------------------------------------------------------
static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset)
{
  channel* currentChannel; 
  char mid_message[BUF_LEN];
  ssize_t i, j;
  //If no channel has been set on the file descriptor
  currentChannel = (channel *)file->private_data;
  if(currentChannel == NULL){
    printk("im hererereerere 11\n");
    return -EINVAL;
  }
  //If the passed message length is 0 or more than 128
  if(length == 0 || length > 128){
    printk("im hererereerere 2\n");
    return -EMSGSIZE;
  }
  if(buffer == NULL){
    printk("im hererereerere 3\n");
    return -EINVAL;
  }
  printk("Invoking device_write(%p,%ld)\n", file, length);
  for( i = 0; i < length && i < BUF_LEN; ++i ) {
    if(get_user(mid_message[i], &buffer[i]) != 0) {
      printk("im hererereerere 4\n");
      return -EINVAL;
    };
  }
  for(j = 0; j<i; ++j){
    currentChannel->messageString[j] = mid_message[j];
  }
  currentChannel->mesLen= i;
  // return the number of input characters used
  printk("what the fuc");
  return i;
}

//---------------------------------------------------------------
// a process which has already opened
// the device file attempts to read from it
static ssize_t device_read( struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
  int i;
  channel *currentChannel;
  currentChannel  = (channel *)file->private_data;
  //If no channel has been set on the file descriptor
  if(currentChannel == NULL){
    return -EINVAL;
  }
  //If no message exists on the channel
  if(currentChannel->mesLen == 0){
    return -EWOULDBLOCK;
  }
  //If the provided buffer length is too small to hold the last message written on the channel
  if(currentChannel->mesLen > length){
    return -ENOSPC;
  }
  if(buffer == NULL){
    return -EINVAL;
  }
  for(i = 0; i < currentChannel->mesLen; i++){
    if (put_user(currentChannel->messageString[i], &buffer[i]) != 0){
      return -EINVAL;
    }
  }
    return currentChannel->mesLen;
}

//==================== DEVICE SETUP =============================

// This structure will hold the functions to be called
// when a process does something to the device we created
struct file_operations Fops = {
  .owner	  = THIS_MODULE, 
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl = device_ioctl,
};

//---------------------------------------------------------------
// Initialize the module - Register the character device
static int __init simple_init(void)
{
  int rc = -1;
  int i;
  // Register driver capabilities. Obtain major num
  rc = register_chrdev( MAJOR_NUM, DEVICE_RANGE_NAME, &Fops );

  // Negative values signify an error
  if( rc < 0 ) {
    printk( KERN_ALERT "registraion failed");
    return -1;
  }
  for(i=0; i<=256; i++){
    massageSlotsDeviceFilesList[i].first = NULL;
  }
  printk("!!!");
    return SUCCESS;
}

//---------------------------------------------------------------
static void __exit simple_cleanup(void)
{
  channel *pointer;
  channel *tmp;
  int i;
  for(i=0; i<=256; i++){
    pointer = massageSlotsDeviceFilesList[i].first;
    while(pointer != NULL){
      tmp = pointer;
      pointer = pointer->next;
      kfree(tmp);
    }
  }
  unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

//---------------------------------------------------------------
module_init(simple_init);
module_exit(simple_cleanup);

//========================= END OF FILE =========================
