'''
Created on Mar 24, 2016

@author: consultit
'''

# from direct.actor.Actor import Actor
import panda3d.core
from panda3d.core import load_prc_file_data
from direct.showbase.ShowBase import ShowBase

dataDir = "../data"

if __name__ == '__main__':
    # Load your application's configuration
    load_prc_file_data("", "model-path " + dataDir)
    load_prc_file_data("", "win-size 1024 768")
    load_prc_file_data("", "show-frame-rate-meter #t")
    load_prc_file_data("", "sync-video #t")
        
    # Setup your application
    app = ShowBase()
       
    # # here is room for your own code
    
    # place camera
    trackball = app.trackball.node()
    trackball.set_pos(-10.0, 90.0, -2.0);
    trackball.set_hpr(0.0, 15.0, 0.0);
   
    # app.run(), equals to do the main loop in C++
    app.run()

