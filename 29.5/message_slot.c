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
  //If the passed command is not MSG_SLOT_CHANNEL, the ioctl() returns -1 and errno is set to EINVAL.
  if(ioctl_command_id != MSG_SLOT_CHANNEL){
    return -EINVAL;
  }
  //If the passed channel id is 0, the ioctl() returns -1 and errno is set to EINVAL.
  if(ioctl_param == 0){
    return -EINVAL;
  }
  curChannelMinorNum = iminor(file->f_inode);
  if (curChannelMinorNum == NULL){
    printk("it is Null");
  }
  channelPointer = massageSlotsDeviceFilesList[curChannelMinorNum].first;
  while(channelPointer != NULL) {
    if(channelPointer->ID == ioctl_param){
        break;
      }
    tmp = channelPointer;
    channelPointer = channelPointer->next;
  }
  // there is no channel with this ID
  if(channelPointer == NULL){
    printk("middle");
    channelPointer = (channel *)kmalloc(sizeof(channel), GFP_KERNEL);
    // failing to allocate memory
    if (channelPointer == NULL) {
      printk("1 after middle");
      return -EINVAL;
    }
    printk("2 after middle");
    if (tmp == NULL) {
      printk("3 after middle");
      massageSlotsDeviceFilesList[curChannelMinorNum].first = channelPointer;
    }
    else {
      printk("5 after middle");
      tmp->next = channelPointer;
    }
    printk("middle2");
    channelPointer->ID = ioctl_param;
    channelPointer->mesLen = 0;
    channelPointer->next = NULL;
  }
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
    return -EINVAL;
  }
  //If the passed message length is 0 or more than 128
  if(length == 0 || length > 128){
    return -EMSGSIZE;
  }
  if(buffer == NULL){
    return -EINVAL;
  }
  printk("Invoking device_write(%p,%ld)\n", file, length);
  for( i = 0; i < length && i < BUF_LEN; ++i ) {
    if(get_user(mid_message[i], &buffer[i]) != 0) {
      printk("im hererereerere 4\n");
      return -EINVAL;
    }
  }
  for(j = 0; j<i; ++j){
    currentChannel->messageString[j] = mid_message[j];
  }
  currentChannel->mesLen= i;
  // return the number of input characters used
  return i;
}

//---------------------------------------------------------------
// a process which has already opened
// the device file attempts to read from it
static ssize_t device_read( struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
  printk("hihihi");
  int i;
  channel *currentChannel;
  printk("hellloooo");
  currentChannel  = (channel *)file->private_data;
  printk("goodbbbb");
  //If no channel has been set on the file descriptor
  if(currentChannel == NULL){
    printk(" read 1");
    return -EINVAL;
  }
  printk("read after 1");
  //If no message exists on the channel
  if(currentChannel->mesLen == 0){
    printk(" read 2");
    return -EWOULDBLOCK;
  }
  printk("read after 2");
  //If the provided buffer length is too small to hold the last message written on the channel
  if(currentChannel->mesLen > length){
    printk(" read 3");
    return -ENOSPC;
  }
  printk("read after 3");
  if(buffer == NULL){
    printk(" read 4");
    return -EINVAL;
  }
  printk("read after 4");
  for(i = 0; i < currentChannel->mesLen; i++){
    printk(" read 5");
    if (put_user(currentChannel->messageString[i], &buffer[i]) != 0){
      return -EINVAL;
    }
  }
  printk("read after 5");
  printk("len is %d", currentChannel->mesLen);
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
  printk("end of init");
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
