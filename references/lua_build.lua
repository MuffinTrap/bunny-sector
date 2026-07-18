-- HELLO FROM MUFFINTRAP
function V2New(x,y)
	return {x=x,y=y}
end

function V2Add(A, B)
	return V2New(A.x+B.x, A.y+B.y)
end

function V2Subtract(A, B)
	return V2New(A.x-B.x, A.y-B.y)
end

function V2Scale(A, s)
	return V2New(A.x*s, A.y*s)
end

function V2Length(A)
	return math.sqrt( (A.x*A.x)+(A.y*A.y))
end

function V2DotProduct(A, B)
	return (A.x*B.x + A.y*B.y)
end

function V2CrossProduct(A, B)
	return (A.x*B.y - A.y*B.x)
end

function V2Distance(A, B)
	return V2Length(V2Subtract(A,B))
end

function V2Normalize(A)
	local l = V2Length(A)
	if l > 0 then
		return V2New(A.x/l,A.y/l)
	else
		return V2New(0,0)
	end
end

function V2Rotate(A, angle)
	local cr = math.cos(angle)
	local sr = math.sin(angle)
	local x = A.x*cr - A.y*sr
	local y = A.x*sr + A.y*cr
	return V2New(x,y)
end

function Intersect(A1,A2,B1,B2)
	local AxA=V2CrossProduct(A1,A2)
	local BxB=V2CrossProduct(B1,B2)
	local xt=AxA*(B1.x-B2.x)-(A1.x-A2.x)*BxB
	local yt=AxA*(B1.y-B2.y)-(A1.y-A2.y)*BxB
	local det=(A1.x-A2.x)*(B1.y*B2.y)-(A1.y-A2.y)*(B1.x-B2.x)
	if det==0 then
		return V2New(0,0)
	end

	return V2New(xt/det,yt/det)
end

function IntersectB(A1,A2,B1,B2)
	local AxA=V2CrossProduct(A1,A2)
	local BxB=V2CrossProduct(B1,B2)
	local toA1=V2Subtract(A1,A2)
	local toB1=V2Subtract(B1,B2)
	local det=V2CrossProduct(toA1,toB1)
	local x=V2CrossProduct( V2New(AxA,toA1.x), V2New(BxB,toB1.x))/det;
	local y=V2CrossProduct( V2New(AxA,toA1.y), V2New(BxB,toB1.y))/det;
	return V2New(x,y)
end

-- Constants
DEG2RAD=(math.pi/180.0)
RAD2DEG=(180.0/math.pi)
SW=240 -- Canvas Width
SH=136 -- Canvas Height
SC=V2New(SW/2,SH/2)
VIEWPORT=V2New(1.0,1.0)--Viewport in world
FOVRAD=90*DEG2RAD -- VERTICAL FOV
HFOVRAD=90*DEG2RAD -- HORIZONTAL FOV
ANGLERAD=0 -- CAMERA YAW
CONVERT=V2New(1,1)--Do CameraToCanvas conversion.Divide this by z

NEARZ=0.1 -- Near plane
CAMERA=V2New(20,40) --Camera/player position

PRINT=false
FILL=true

function lineV(A,B,col)
	line(A.x,A.y,B.x,B.y,col)
end

GUI_Y=8
function resety()
	GUI_Y=8
end
function nexty()
	GUI_Y=GUI_Y+8
	return GUI_Y
end

-- Transform point from Viewport to Canvas
function ViewToCanvas(P)
	return V2New(P.x*SW/VIEWPORT.x,P.y*SH/VIEWPORT.y)
end

-- Transform point from Camera space to Viewport
function CameraToView(P,z)
	return V2New((P.x*NEARZ)/z,(P.y*NEARZ)/z)
end

function ProjectVertex(P,z)
	return ViewToCanvas(WorldToView(P,z))
end

-- Calculate Height of viewport from Vertical
-- fov and near plane
function ViewHeightFromFOVNEAR(fovrad,nearz)
	local viewh=math.tan(fovrad/2)*nearz
	return viewh*2
end

-- Calculate width of viewport from Horizontal
-- fov and near plane
function ViewWidthFromFOVNEAR(fovrad,nearz)
	local vieww=math.tan(fovrad/2)*nearz
	return vieww*2
end

function WorldToCamera(Point,Camerapos,camerayaw)
	local tP=V2Subtract(Point,Camerapos)
	local rP=V2Rotate(Point,-camerayaw)
	return rP
end

-- When FOV changes, calculate
-- everything
function GetConvertXY()
	local aspect=SW/SH
	HFOVRAD=FOVRAD*aspect

	VIEWPORT=V2New(
					ViewWidthFromFOVNEAR(HFOVRAD,NEARZ),
							ViewHeightFromFOVNEAR(FOVRAD,NEARZ))

	local tx=(NEARZ)*SW/VIEWPORT.x
	local ty=(NEARZ)*SW/VIEWPORT.y
	return V2New(tx,ty)
end

-- X is to sid
-- Y is to up
-- Z is to away
function CameraToScreen(x,y,z)
	-- normalized device coordinates
	local ndc = V2New((x*NEARZ)/z,(y*NEARZ)/z)
	local canvas = V2New(ndc.x*SW/VIEWPORT.x,ndc.y*SH/VIEWPORT.y)
	local screen = V2New(SC.x+canvas.x,SC.y+canvas.y)

	local tx=(x*NEARZ/z)*SW/VIEWPORT.x
	local ty=(y*NEARZ/z)*SW/VIEWPORT.y

end

function fovtest()

	print("FOV Dg:"..RAD2DEG*FOVRAD,8,nexty(),5)
 --Forward
	lineV(SC,V2Add(SC,V2New(100,0)),10)
	-- Fov limit
	local fovline=V2Rotate(V2New(100,0),-FOVRAD/2)
	lineV(SC,V2Add(SC,fovline),4)
	local nearpoint=V2New(NEARZ,0)
	lineV(V2Add(SC,nearpoint),V2New(SC.x+nearpoint.x,0),12)

	local clipP=IntersectB(
		nearpoint,V2Add(nearpoint,V2New(0,-10)),
		V2New(0,0),fovline)
	print("Clip"..clipP.x..","..clipP.y,8,nexty(),4)
	circb(SC.x+clipP.x,SC.y+clipP.y,2,4)

	local clipd=V2Distance(clipP,V2New(NEARZ,0))
	print("ClipD: "..clipd,8,nexty(),6)

	-- Calculate half of viewport
	-- tan(fov/2)=vievh/nearz
	local viewh=math.tan(FOVRAD/2)*NEARZ
	print("VievH: " .. viewh,8,nexty(),6)
end

function intersecttest(rot)
	local p1=V2New(-40,-10)
	local p2=V2Add(V2New(-40,-10),V2Rotate(V2New(10,0),rot))

	local p3=V2New(-10,-5)
	local p4=V2New(-15,10)

	lineV(V2Add(SC,p1),V2Add(SC,p2),2)
	lineV(V2Add(SC,p3),V2Add(SC,p4),4)
	local ip=Intersect(p1,p2,p3,p4)
	local ipB=IntersectB(p1,p2,p3,p4)
	circb(SC.x+ip.x,SC.y+ip.y,2,12)
	circb(SC.x+ipB.x,SC.y+ipB.y,2,5)
end

function drawscene()
	local wall_list={
		V2New(30,20),
		V2New(40,-5),
		V2New(60,-5),
		V2New(70,0),
		V2New(65,10),
		V2New(50,30),

		V2New(30,70),
		V2New(-20,70),
		V2New(-20,20)
	}
	-- TOP DOWN
	-- wall
	for i=1,#wall_list do
	local A=wall_list[i]
	local nexti=i+1
	if nexti > #wall_list then
		nexti=1
	end
	local B=wall_list[nexti]
--	lineV(A,B,12)
 -- player
	local rP=V2Rotate(V2New(10,0),ANGLERAD)

--	lineV(CAMERA,V2Add(rP,CAMERA),8)
--	pix(CAMERA.x,CAMERA.y,12)

	-- ROTATED TOPDOWN
	-- Note: Around player means negative
	-- Angles
	local tA=V2Subtract(A,CAMERA)
	local tB=V2Subtract(B,CAMERA)
	tA=V2Rotate(tA,-ANGLERAD)
	tB=V2Rotate(tB,-ANGLERAD)
	if i==1 and PRINT then
	print("Rotated",8,nexty(),12)
	print(string.format("A.x %.2f %.2f",tA.x,tA.y),8,nexty(),4)
	print(string.format("B.x %.2f %.2f",tB.x,tB.y),8,nexty(),5)
	end

	-- draw wall
	local cA=V2Add(SC,tA)
	local cB=V2Add(SC,tB)
	lineV(cA,cB,13)

	if i==1 and PRINT then
		circ(cA.x,cA.y,2,4)
		circ(cB.x,cB.y,2,5)
		-- draw player
		lineV(SC, V2Add(SC,V2New(10,0)),8)
		pix(SC.x,SC.y,12)
	end

	-- PERSPECTIVE
	local wallheight=10

	-- Rotated point:x is in front of player
	-- Negative Y is to left
	-- Positive Y is to right
	-- Player view frustum from top
	local lcs=V2New(0.01,-0.01)
 local rcs=V2New(0.01,0.01)
	local lcp=V2Rotate(V2New(20,0),-HFOVRAD/2)
	local rcp=V2Rotate(V2New(20,0),HFOVRAD/2)
	lineV(V2Add(SC,lcs),V2Add(SC,lcp),6)
	lineV(V2Add(SC,rcs),V2Add(SC,rcp),10)

	-- Calculate clip points against frustum
	local c1=IntersectB(tA,tB,lcs,lcp)
	local c2=IntersectB(tA,tB,rcs,rcp)
	if i==1 then
	if c1.x>0 and PRINT then
		circ(SC.x+c1.x,SC.y+c1.y,2,6)
		lineV(SC,V2Add(SC,c1),6)

	end
	if c2.x>0 and PRINT  then
		circ(SC.x+c2.x,SC.y+c2.y,2,10)
		lineV(SC,V2Add(SC,c2),10)
	end
	if PRINT then
		print("Clips",8,nexty(),12)
		print(string.format("C1 x:%.2f y:%.2f",c1.x,c1.y),16,nexty(),6)
		print(string.format("C2 x:%.2f y:%.2f",c2.x,c2.y),16,nexty(),10)
	end
 end
	-- Draw or reject the wall
	local draw=true

	if tA.x<0 and tB.x<0 then
		-- Whole wall is behind
		draw=false
	else
		-- If either point is behind,
		-- find correct clip point
		if draw and (tA.x<0 or tB.x<0) then


			if tA.x<0 then
				if c1.x>0 then
						if i==1 and PRINT then
							print("Clip: A to c1",8,nexty(),6)
							lineV(V2Add(SC,tA),V2Add(SC,c1),6)
						end
					tA=c1
				else
					draw=false
				end
			end

			if tB.x<0 then
				if c2.x>0 then
					if i==1 and PRINT then
						print("Clip: B to c2",8,nexty(),10)
						lineV(V2Add(SC,tB),V2Add(SC,c2),10)
					end
					tB=c2
				else
				 draw=false
				end
			end
		end --End clip

		if draw then
			-- From camera's view
			local az=tA.x -- Distance to point
			local bz=tB.x
			local ax=tA.y -- Sideways distance
			local bx=tB.y
			local ayt=-wallheight/2--top
			local byt=-wallheight/2
			local ayb=wallheight/2 --bottom
			local byb=wallheight/2

			-- Transform from World to Viewport
			local Awt=CameraToView(V2New(ax,ayt),az)
			local Bwt=CameraToView(V2New(bx,byt),bz)
			local Awb=CameraToView(V2New(ax,ayb),az)
			local Bwb=CameraToView(V2New(bx,byb),bz)
			if i==1 and PRINT then
			print(string.format("AinView (%.2f,%.2f",Awt.x,Awt.y),8,nexty(),4)
		 print(string.format("BinView (%.2f,%.2f",Bwt.x,Bwt.y),8,nexty(),5)
			end

			-- Transform from Viewport to Canvas
			local Act=ViewToCanvas(Awt)
			local Bct=ViewToCanvas(Bwt)
			local Acb=ViewToCanvas(Awb)
			local Bcb=ViewToCanvas(Bwb)
			if i==1 and PRINT then
			print(string.format("AinCanv (%.2f,%.2f",Act.x,Act.y),8,nexty(),4)
		 print(string.format("BinCanv (%.2f,%.2f",Bct.x,Bct.y),8,nexty(),5)
			end

			-- Check before drawing that
			-- wall still goes from left to right
			if Act.x<Bct.x then

				-- Translate origo to center of canvas
				local Amt=V2Add(SC,Act)
				local Bmt=V2Add(SC,Bct)
				local Amb=V2Add(SC,Acb)
				local Bmb=V2Add(SC,Bcb)

				-- Finally draw the wall edges
				lineV(Amt,Bmt,13)
				lineV(Amb,Bmb,13)
				lineV(Amt,Amb,4)
				lineV(Bmt,Bmb,5)
				if i==1 and PRINT then
					circ(Amt.x,Amt.y,2,4)
					circ(Bmt.x,Bmt.y,2,5)
				end

				-- Check if convert matches
				local CAt=V2Scale(V2New(CONVERT.x*ax,CONVERT.y*ayt),1.0/az)
				local CBt=V2Scale(V2New(CONVERT.x*bx,CONVERT.y*byt),1.0/bz)
				CAt=V2Add(SC,CAt)
				CBt=V2Add(SC,CBt)
				lineV(Amt,Bmt,12)
			end -- not back
		end -- draw
	end
 end -- wall loop
end

function AdjustFOV()
 if btn(4) then
		if btn(0) then
			FOVRAD = FOVRAD+0.01
		end
		if btn(1) then
			FOVRAD = FOVRAD-0.01
		end

	else
		if btn(0) then
			HFOVRAD = HFOVRAD+0.01
		end
		if btn(1) then
			HFOVRAD = HFOVRAD-0.01
		end
	end

	print(string.format("V FOV: %.2f",RAD2DEG*FOVRAD),8,nexty(),12)
	print(string.format("H FOV: %.2f",RAD2DEG*HFOVRAD),8,nexty(),12)
end

function AdjustANGLE()
	if btn(2) then
		ANGLERAD = ANGLERAD+0.01
	end
	if btn(3) then
		ANGLERAD = ANGLERAD-0.01
	end
	print("Angle: "..RAD2DEG*ANGLERAD,8,nexty(),12)
end

function AdjustCAMERA()
	local dir=V2Rotate(V2New(1.0,0.0),ANGLERAD)
	local speed=0
	if btn(0) then
		speed=1
	elseif btn(1) then
		speed=-1
	end

	CAMERA=V2Add(CAMERA,V2Scale(dir,speed))
end

function AdjustNEARZ()
	if btn(3) then
		NEARZ = NEARZ+1.01
	end
	if btn(2) then
		NEARZ = NEARZ-1.01
		if NEARZ<0.01 then
			NEARZ=0.01
		end
	end
	print("Nearz"..NEARZ,8,nexty(),5)
end

function TIC()t=time()//32
	cls(1)
	resety()


	if btn(5) then
		AdjustFOV()
	else
	AdjustANGLE()
	AdjustCAMERA()
	end

	VIEWPORT=V2New(
					ViewWidthFromFOVNEAR(HFOVRAD,NEARZ),
							ViewHeightFromFOVNEAR(FOVRAD,NEARZ))
			if PRINT then
	print(string.format("Viewport: %.2f,%.2f",VIEWPORT.x,VIEWPORT.y),8,nexty(),12)
	end
 CONVERT=GetConvertXY()
 if PRINT then
 print(string.format("Convert X:%.2f Y:%.2f",CONVERT.x,CONVERT.y),8,nexty(),11)
end


--	fovtest()
	drawscene()
end
