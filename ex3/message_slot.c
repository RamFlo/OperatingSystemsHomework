#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "message_slot.h"

MODULE_LICENSE("GPL");

minorMsgSlotNode* headNode=NULL;

minorMsgSlotNode *findMinorInSlotList(minorMsgSlotNode *curNode, int minorToFind)
{
  if (curNode == NULL)
    return NULL;
  if (curNode->minorNumber == minorToFind)
    return curNode;
  return findMinorInSlotList(curNode->next, minorToFind);
}

int createAndAddNewMinorSlot(int minorToAdd)
{
  minorMsgSlotNode *newMinorNode = (minorMsgSlotNode *)kmalloc(sizeof(minorMsgSlotNode), GFP_KERNEL);
  if (newMinorNode == NULL)
    return -1;
  newMinorNode->minorNumber = minorToAdd;
  newMinorNode->closed = 0;
  //newMinorNode->channel=-1;
  newMinorNode->next = headNode;
  headNode = newMinorNode;
  return SUCCESS;
}

void freeMsgListMem(minorMsgSlotNode *curNode)
{
  if (curNode == NULL)
    return;
  freeMsgListMem(curNode->next);
  kfree(curNode);
}

//================== DEVICE FUNCTIONS ===========================
static int device_open(struct inode *inode, struct file *file)
{
  int minorToOpen = iminor(inode);
  minorMsgSlotNode *minorPtr = findMinorInSlotList(headNode, minorToOpen);
  if (minorPtr != NULL)
  {
    minorPtr->closed = 0;
    //minorPtr->channel=-1;
    return SUCCESS;
  }
  if (createAndAddNewMinorSlot(minorToOpen) < 0)
    return -ENOMEM;
  return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
  int minorToRelease = iminor(inode);
  minorMsgSlotNode *minorPtr = findMinorInSlotList(headNode, minorToRelease);
  if (minorPtr == NULL)
  {
    minorPtr->closed = 1;
    return SUCCESS;
  }
  return -EINVAL;
}

//---------------------------------------------------------------
// a process which has already opened
// the device file attempts to read from it
static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
  return -EINVAL;
}

// a processs which has already opened
// the device file attempts to write to it
static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset)
{
  int minorNum=-1,i=0,channelNum=-1;
  minorMsgSlotNode *minorPtr=NULL;
  //no channel has been set
  if (file->private_data==NULL)
    return -EINVAL;
    //given message length surpasses limit
  if (length>MSG_SIZE)
    return -EINVAL;
  minorNum=iminor(file_inode(file));
  minorPtr = findMinorInSlotList(headNode, minorNum);
  //write requested for a device that never invoked device_open()
  if (minorPtr==NULL)
    return -EINVAL;
  //write requested for a closed device
  if (minorPtr->closed==1)
    return -EINVAL;
  channelNum=(int)file->private_data;
  printk("Invoking device_write(%p,%d)\n", file, length);
  for (i = 0; i < length; i++)
  {
    if(get_user(minorPtr->messageSlotArray[channelNum][i], &buffer[i])<0)
      return -EINVAL;
  }
  // return the number of input characters used
  return i;
}

static long device_ioctl(struct file *file, unsigned int ioctl_command_id, unsigned long ioctl_param)
{
  if (ioctl_command_id != MSG_SLOT_CHANNEL)
    return -EINVAL;
  if (ioctl_param<0 || ioctl_param>=SLOT_CHANNELS)
    return -EINVAL;
  file->private_data=(void*)ioctl_param;
  return SUCCESS;
}
//==================== DEVICE SETUP =============================

// This structure will hold the functions to be called
// when a process does something to the device we created
struct file_operations Fops =
{
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl = device_ioctl,
  .release        = device_release,
};

//---------------------------------------------------------------
// Initialize the module - Register the character device
static int __init msg_slot_init(void)
{
  int majorNum=-1;
  // Register driver capabilities. Obtain major num
  majorNum = register_chrdev( MAJOR_NUM, DEVICE_RANGE_NAME, &Fops );
  if( majorNum < 0 )
  {
    printk( KERN_ALERT "%s registration failed for major number %d\n",DEVICE_RANGE_NAME,MAJOR_NUM);
    return majorNum;
  }
  printk(KERN_INFO "message_slot: registered major number %d\n", majorNum);
  return SUCCESS;
}

//---------------------------------------------------------------
static void __exit msg_slot_cleanup(void)
{
  freeMsgListMem(headNode);
  unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

//---------------------------------------------------------------
module_init(msg_slot_init);
module_exit(msg_slot_cleanup);

//========================= END OF FILE =========================