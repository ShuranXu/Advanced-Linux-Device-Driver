#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/delay.h>
#include <linux/io.h>

// #include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/dma.h>

static DEFINE_SPINLOCK(memblock_lock);                  /*定义自旋锁*/
static struct request_queue  *memblock_request;              /*申请队列*/
static struct gendisk   *memblock_disk;                 /*磁盘结构体*/
static int memblock_major;

#define BLOCKBUF_SIZE               (1024*1024)         /*磁盘大小*/
#define SECTOR_SIZE                   (512)             /*扇区大小*/
static unsigned char   *block_buf;                      /*磁盘地址*/


static int memblock_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{    
    geo->heads =2;                                         // 2个磁头分区
    geo->cylinders = 32;                                   //一个磁头有32个柱面
    geo->sectors = BLOCKBUF_SIZE/(2*32*SECTOR_SIZE);      //一个柱面有多少个扇区    
    return 0;
}

static struct block_device_operations memblock_fops = {
       .owner    = THIS_MODULE,
       .getgeo   =  memblock_getgeo,                //几何,保存磁盘的信息(柱头,柱面,扇区)
};

/*申请队列处理函数*/
static void do_memblock_request (struct request_queue * q)
{
        struct request *req;
        unsigned long offset;
        unsigned long len; 
        static unsigned long r_cnt = 0;
        static unsigned long w_cnt = 0;
              
        while ((req = elv_next_request(q)) != NULL)        //获取每个申请
        {
            offset=req->sector*SECTOR_SIZE;                     //偏移值
            len=req->current_nr_sectors*SECTOR_SIZE;            //长度    
                        
            /* 获取request申请结构体的命令标志(cmd_flags成员),
            * 当返回READ(0)表示读扇区命令,否则为写扇区命令 
            */
            if(rq_data_dir(req)==READ)
            {            
                memcpy(req->buffer,block_buf+offset,len);       //读出缓存
            }
            else
            {              
                memcpy(block_buf+offset,req->buffer,len);     //写入缓存
            }
            end_request(req, 1);                             //结束获取的申请
        }    
}

/*入口函数*/
static int memblock_init(void)
{
     /*1)使用register_blkdev()创建一个块设备*/
     memblock_major=register_blkdev(0, "memblock");     
     
     /*2) blk_init_queue()使用分配一个申请队列,并赋申请队列处理函数*/
     memblock_request=blk_init_queue(do_memblock_request,&memblock_lock);
    
     /*3)使用alloc_disk()分配一个gendisk结构体*/
     memblock_disk=alloc_disk(16);                        //不分区
    
     /*4)设置gendisk结构体的成员*/
     /*->4.1)设置成员参数(major、first_minor、disk_name、fops)*/           
     memblock_disk->major = memblock_major;
     memblock_disk->first_minor = 0;
     sprintf(memblock_disk->disk_name, "memblock");
     memblock_disk->fops = &memblock_fops;
        
     /*->4.2)设置queue成员,等于之前分配的申请队列*/
     memblock_disk->queue = memblock_request;
      
     /*->4.3)通过set_capacity()设置capacity成员,等于扇区数*/
     set_capacity(memblock_disk,BLOCKBUF_SIZE/SECTOR_SIZE);
   
     /*5)使用kzalloc()来获取缓存地址,用做扇区*/
     block_buf=kzalloc(BLOCKBUF_SIZE, GFP_KERNEL);
 
     /*6)使用add_disk()注册gendisk结构体*/
     add_disk(memblock_disk);   
     return  0;
}
static void memblock_exit(void)
{        
      /*1)使用put_disk()和del_gendisk()来注销,释放gendisk结构体*/

      /* ->1.1)注销内核中的gendisk结构体 */
      put_disk(memblock_disk);
      /* ->1.2)释放gendisk结构 */
      del_gendisk(memblock_disk);

      /*2)使用kfree()释放磁盘扇区缓存   */ 
      kfree(block_buf);

      /*3)使用blk_cleanup_queue()清除内存中的申请队列    */
      blk_cleanup_queue(memblock_request);
      
      /*4)使用unregister_blkdev()卸载块设备               */
      unregister_blkdev(memblock_major,"memblock");
}

module_init(memblock_init);
module_exit(memblock_exit);
MODULE_LICENSE("GPL");
