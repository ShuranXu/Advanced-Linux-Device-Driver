cmd_/home/shuran/Advanced-Linux-Device-Driver/Ground-Work/Assignment1/Module.symvers := sed 's/ko$$/o/' /home/shuran/Advanced-Linux-Device-Driver/Ground-Work/Assignment1/modules.order | scripts/mod/modpost -m -a   -o /home/shuran/Advanced-Linux-Device-Driver/Ground-Work/Assignment1/Module.symvers -e -i Module.symvers   -T -
