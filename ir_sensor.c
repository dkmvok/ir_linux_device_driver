/* Kernel */
#define GPIO_TRIGGER 23
#define GPIO_OUT  3

struct cdev *ir_cdev;
int drv_mjr = 0;

struct file_operations ir_dev_fops = {
	.owner =     THIS_MODULE,
	.read =	     ir_dev_read,
	.open =	     ir_dev_open,
	.release =   ir_dev_release
};

static int ir_dev_init(void)
{
    int retVal;
    dev_t dev = MKDEV(drv_mjr, 0);

    pr_info("ir_cdev: initializing ir dev\n");

    retVal = alloc_chrdev_region(&dev, 0, 1, "ir_cdev");
    drv_mjr = MAJOR(dev);

    if (retVal < 0) {
        pr_alert("ERROR ir_cdev:  alloc_chrdev_region\n");
        return retVal;
    }

    ir_cdev = cdev_alloc();
    ir_cdev->ops = &ir_dev_fops;

    retVal = cdev_add(ir_cdev, dev, 1);
    if (retVal < 0) {
        pr_alert("Error ir_cdev: cdev_add\n");
        unregister_chrdev_region(dev, 1);
        return retVal;
    }

    if (gpio_is_valid(GPIO_TRIGGER) == false) {
        pr_err("ERROR ir_cdev: %d is not a gpio\n", GPIO_TRIGGER);
        return -1;
    }

    if (gpio_is_valid(GPIO_OUT) == false) {
        pr_err("ERROR ir_cdev: %d is not a gpio\n", GPIO_OUT);
        return -1;
    }

    if (gpio_request(GPIO_TRIGGER, "GPIO_TRIGGER") < 0) {
        pr_err("ERROR ir_cdev: Request for gpio %d\n", GPIO_TRIGGER);
        gpio_free(GPIO_TRIGGER);
        return -1;
    }

    if (gpio_request(GPIO_OUT, "GPIO_OUT") < 0) {
        pr_err("ERROR ir_cdev: Request for gpio %d\n", GPIO_OUT);
        gpio_free(GPIO_OUT);
        return -1;
    }
}

static void ir_dev_exit(void)
{
    dev_t dev = MKDEV(drv_mjr, 0);
    cdev_del(ir_cdev);
    unregister_chrdev_region(dev, 1);

    gpio_set_value(GPIO_TRIGGER, 0);
    gpio_free(GPIO_TRIGGER);
    gpio_free(GPIO_OUT);
    pr_info("ir_cdev: exiting ir dev\n");

}
