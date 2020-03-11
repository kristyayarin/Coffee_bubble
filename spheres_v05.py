#!/usr/bin/env python

# Many thanks to Julian Fong,
#    http://graphics.pixar.com/library/indexAuthorFong.html 
# for supplying the code for this example helper app. Information about "PRMan
# for Python" can be found here,
#    https://renderman.pixar.com/resources/RenderMan_20/prmanForPython.html
# Specific information about the names of constants can be found here,
#    PATH/TO/RenderManProServer-22.X/include/ri.h
# Malcolm Kesson: March 4th 2017

import prman
import sys
import string
from random import uniform, seed
import math
import perlin

ri = prman.Ri()

while True:
	try:
		line = raw_input().strip() #raw_input:wait prman to give a input
	except EOFError:
		break
	else:
		detail, data = line.split(' ', 1);
		inputs = data.split()
		# Data; "5000 4.5 2.0 0.02 0.1 0.5 0.15"
		num = int(inputs[0])
		outerR = float(inputs[1])
		interR = float(inputs[2])
		minrad = float(inputs[3])
		maxrad = float(inputs[4])
		maxform = float(inputs[5])
		threshold = float(inputs[6])
		
		
		circles = []
		
		blob_xyz = []
		blob_codes = []
		
		seed(1)
	#-----------------------Start----------------------------
		# Open a rib stream
		ri.Begin("-")
		count = -1
		bnum = 0
		cform = (maxrad - minrad) * maxform + minrad
		
	#--------------------Creat bubbles----------------------------
		
	while count < num:
		x = uniform(-outerR,outerR)
		z = uniform(-outerR,outerR)
		dist = math.sqrt(x * x + z * z)
		
		jitter = perlin.pnoise(x,0,z)
		
		if dist <= outerR and dist >= interR - jitter: 
			
			gapseed = abs((outerR-dist)-(dist-(interR - jitter)))
			gap = 1 - gapseed / abs(outerR - interR + jitter)
			# Use this parameter to make the bubbles that near the center bigger
		
			size = uniform(minrad, maxrad) * gap
			count +=1
			
			if count == 0:
				circle = (x,z,size)
				circles.append(circle)
				# Save bubble info
				ssize = size+minrad
				
				if size <= cform:
					blob_xyz.extend( [ssize,0,0,0, 0,ssize,0,0, 0,0,ssize,0, x,0,z,1] )
					bnum +=1
                                # Small bubbles will blend together and become form
					
				else:
					ri.TransformBegin()
					ri.Translate(x,0 , z)
					ri.Sphere(ssize,-ssize, ssize, 360)
					ri.TransformEnd()
				# Create big bubbles
				
			else:
				for circle in circles:
					cx = circle[0]
					cz = circle[1]
					cs = circle[2]

					cdist = math.sqrt(math.pow(cx-x,2)+math.pow(cz-z,2))
					# Determine if bubbles are coincident
					cdd = cdist - (cs + size)
				
					if cdist<=cs :
						break
				
					if cdd <= 0 :
						ddsize = cdist - cs - minrad
						size = cdist - cs - uniform(0,ddsize)
					# Keep bubbles from overlapping

					if size<=minrad :
						break

				else:
					if size <= cform:
						blob_xyz.extend( [ssize,0,0,0, 0,ssize,0,0, 0,0,ssize,0, x,0,z,1] )
						bnum +=1
						
					else:
						circle = (x,z,size)
						circles.append(circle)
						ssize = size+minrad
		
						ri.TransformBegin()
						ri.Translate(x, 0, z)
						ri.Sphere(ssize,-ssize, ssize, 360)
						ri.TransformEnd()


	#--------------------Creat form----------------------------
						
	for n in range(bnum):
		blob_codes.extend( (1001, n * 16) )
		
	blob_codes.append(0)
	
	blob_codes.append(bnum)
	
	for n in range(bnum):
		blob_codes.append(n)
		
	ri.Blobby(bnum, blob_codes, blob_xyz, [], { "constant float __threshold" : threshold })
	
	#--------------------Finish----------------------------
		
	# The "/377" escape sequence tells prman we have finished.
	ri.ArchiveRecord(ri.COMMENT, "\n\377")
	sys.stdout.flush

	# Close the rib stream
	ri.End()
