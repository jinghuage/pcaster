#!/usr/bin/env python
# encoding: utf-8
"""
pcaster_gui.py

Created by JGe on 5/27/2010.
Copyright (c)  Jinghua@CCT. All rights reserved.
"""

import sys
import os
import commands
import socket
import string

sys.path[:0] = ['./']

from Tkinter import *
import tkMessageBox
import tkFileDialog
import tkColorChooser


#global vars: dictionaries as cofiguration for renderer, data server, etc. 

pcaster_config = {
"shift":11.5,
"peakscale":2.7,
"sharpness":0.7,
"intensity":0.05,
"colorizer_r":8.2,
"colorizer_g":1.4,
"colorizer_b":3.9
}

def show_pcaster_config():
    pcaster_config_string = "pcaster configuration: \n"

    for key in pcaster_config.keys():
        config_tuple = key + ":\t" + str(pcaster_config[key]) + "\n"
        pcaster_config_string += config_tuple

    info_center.show_info(pcaster_config_string)


def send_pcaster_config():
    pcaster_config_string = ""

    for key in pcaster_config.keys():
        config_tuple = key + " = " + str(pcaster_config[key]) + ","
        pcaster_config_string += config_tuple

    event_server.send(pcaster_config_string.ljust(1024,'\0'))



class Server:
    def __init__(self):
        self.host = 'localhost'
        self.port = 58001
        self.backlog = 5
        self.size = 1024
        self.server = None
        self.client = None
        self.threads = []

    def open_socket(self):
        try:
            self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.server.bind((self.host,self.port))
            self.server.listen(5)
        except socket.error, (value,message):
            if self.server:
                self.server.close()
            print "Could not open socket: " + message
            sys.exit(1)

    def connect(self):
        self.open_socket()
        self.client, address = self.server.accept()
        print "python socket server: client connected", address

    def send(self, event):
        self.client.send(event)

    def close(self):
        self.client.close() 
        self.server.close()



def yesorno():
    if tkMessageBox.askokcancel("Quit", "Do you really want to quit?"):
        root.destroy()


def notdone():
    showerror('Not implemented', 'Not yet available')	

        
def run_pcaster():
    # cmd = 'ls -l ./ofp'
    # os.system(cmd)
    # ret = commands.getstatusoutput("ls -l /Users/jinghua/pcaster/projs/parallel_ofp/render")
    # print ret[1]
    # info_center.show_info(ret[1])

    # os.system("ssh -f localhost /home/jinghuage/pcaster/projs/compositor/ofp")
    # os.system("/Users/jinghua/pcaster/projs/parallel_ofp/render &")
    os.system("/home/jinghuage/pcaster/projs/parallel_ofp/render &")
    event_server.connect()




class pcasterConfigDialog:
    def __init__(self, parent):
        self.parent = parent
        self.shift = DoubleVar()
        self.peakscale = DoubleVar()

    def open(self):
        self.top = Toplevel(self.parent)

        f = Frame(self.top)

        scale = Scale(f, 
                      variable = self.shift, 
                      from_ = 11.0, 
                      to = 12.0,
                      resolution = 0.1,
                      label = "Shift",
                      orient = HORIZONTAL,
                      command = self.set_shift )
        scale.pack(side=LEFT)

        scale = Scale(f, 
                      variable = self.peakscale, 
                      from_ = 0.0, 
                      to = 1.0,
                      resolution = 0.1,
                      label = "PeakScale",
                      orient = HORIZONTAL,
                      command = self.set_peakscale )
        scale.pack(side=LEFT)


        f.pack(side=TOP, fill=X)

        b = Button(self.top, text="close", command=self.close)
        b.pack(side=LEFT,padx=2,pady=2)


    def set_shift(self, value):
        pcaster_config["shift"] = value
        config_string = "%s = %s" % ("shift", value)        
        event_server.send(config_string.ljust(1024,'\0'))

    def set_peakscale(self, value):
        pcaster_config["peakscale"] = value

    def close(self):
        self.top.destroy()

    def validate(self):
        pass


class informationHub:
    def __init__(self, parent):
        f = Frame(parent)

        scrollbar = Scrollbar(f)
        scrollbar.pack( side = RIGHT, fill=Y )
        
        #self.sv = StringVar()
        self.msg = Text(f, 
                   yscrollcommand = scrollbar.set)

        self.msg.pack(side=BOTTOM, expand=YES, fill=BOTH)
        self.msg.config(relief=SUNKEN, width=40, height=7, bg="beige")
        # self.sv.set("Information")

        scrollbar.config( command = self.msg.yview )

        f.pack(side=BOTTOM, fill=X)

    def show_info(self, info):
        config_string=""
        config_string += info
        # self.sv.set(config_string)
        self.clear()
        self.msg.insert(INSERT, config_string)

    def clear(self):
        self.msg.delete("0.0", END)
        # self.sv.set("")


#-----------------------------------------------------------------------------
# main
#-----------------------------------------------------------------------------
if __name__ == '__main__':
    root = Tk()
    root.title('pcaster control center')
        
    info_center = informationHub(root)
    event_server = Server()

    # config toolbar
    config_toolbar = Frame(root)
    b = Button(config_toolbar, text="config viewer", 
               width=12, 
               command=pcasterConfigDialog(root).open,
               state=DISABLED)
    b.pack(side=LEFT, padx=2, pady=2)

    b = Button(config_toolbar, text="config renderer", 
               width=12, 
               command=pcasterConfigDialog(root).open)
    b.pack(side=LEFT, padx=2, pady=2)


    b = Button(config_toolbar, text="config data server ", width=12, 
               command=pcasterConfigDialog(root).open,
               state=DISABLED)
    b.pack(side=LEFT, padx=2, pady=2)
    config_toolbar.pack(side=TOP, fill=X)

    # run toolbar
    run_toolbar = Frame(root)
    b = Button(run_toolbar, text="run viewer", 
               width = 12, 
               command = notdone,
               state = DISABLED)
    b.pack(side=LEFT, padx=2, pady=2)

    b = Button(run_toolbar, text="run renderer", 
               width = 12, 
               command = run_pcaster,
               state = NORMAL)
    b.pack(side=LEFT, padx=2, pady=2)


    b = Button(run_toolbar, text="run data server ", 
               width = 12, 
               command = notdone,
               state = DISABLED)
    b.pack(side=LEFT, padx=2, pady=2)
    run_toolbar.pack(side=TOP, fill=X)

    # control toolbar
    control_toolbar = Frame(root)
    b = Button(control_toolbar, text="show config", 
               width=12,     
               command=show_pcaster_config,
               state=NORMAL)
    b.pack(side=LEFT, padx=2, pady=2)

    b = Button(control_toolbar, text="send config", 
               width=12,     
               command=send_pcaster_config,
               state=NORMAL)
    b.pack(side=LEFT, padx=2, pady=2)


    b = Button(control_toolbar, text="quit", width=12, command=root.quit)
    b.pack(side=LEFT, padx=2, pady=2)
    control_toolbar.pack(side=TOP, fill=X)

    #main loop
    root.protocol("WM_DELETE_WINDOW", yesorno)
    root.mainloop()

