/* Geometry and vector functions for the game*/

#include "OR.h"

extern Player ship[2];
extern Level level;
extern ofstream dbout;


//Converts an absolute floating-point position to a pixel position on screen in integers 
//for the given player, returns whether it's on screen or not
bool PosToScreen(const t_vector *Pos, t_point *Screen, int player)
{
	t_vector offset;

	offset.x = Pos->x - ship[player].position.x;
	offset.y = Pos->y - ship[player].position.y;

	//Shift - to create infinitely wrapping levels
	if(offset.x>level.hwidth) offset.x -= level.fwidth;		//Shift to the left domain
	else if(offset.x<level.mhwidth) offset.x += level.fwidth;	//Shift to the right domain

	if(offset.y>level.hheight) offset.y -= level.fheight;		//Shift to the left domain
	else if(offset.y<level.mhheight) offset.y += level.fheight;	//Shift to the right domain

	Screen->x = (int)(ship[player].screenscale*(offset.x*ship[player].cosangle - offset.y*ship[player].sinangle) + ship[player].scrposition.x);
	Screen->y = (int)(ship[player].screenscale*(offset.x*ship[player].sinangle + offset.y*ship[player].cosangle) + ship[player].scrposition.y);

	if(Screen->x<ship[player].scrtopleft.x) return false;
	if(Screen->x>ship[player].scrbottomright.x) return false;
	if(Screen->y<ship[player].scrtopleft.y) return false;
	if(Screen->y>ship[player].scrbottomright.y) return false;

	return true;
}

//Converts a pixel position on screen in integers to an absolute position in floating point for given player
void ScreenToPos(t_vector *Pos, const t_point *Screen, int player)
{
	t_vector Offset;

	Offset.x = (float)(Screen->x - ship[player].scrposition.x)/ship[player].screenscale;		//Calculate position relative to centre of rotation
	Offset.y = (float)(Screen->y - ship[player].scrposition.y)/ship[player].screenscale;

	Pos->x = Offset.x*ship[player].cosangle + Offset.y*ship[player].sinangle   + ship[player].position.x;
	Pos->y = -Offset.x*ship[player].sinangle + Offset.y*ship[player].cosangle  + ship[player].position.y;

	return;
}

//vectors in the pattern H = (x,0,z), V = (0,y,z)
t_3vector CrossProduct(t_3vector H, t_3vector V)
{
	t_3vector normal;
	normal.x = H.y*V.z - V.y*H.z;
	normal.y = H.x*V.z - V.x*H.z;
	normal.z = H.x*V.y - V.x*H.y;
	return normal;
}

//Calculate the normalised version of an input vector - returns the magnitude/distance
float Normalise(t_vector *in)
{
	float ds = in->x*in->x + in->y*in->y;
	float a = magic_inv_sqrt(ds);
	in->x = in->x*a;
	in->y = in->y*a;
	return ds*a;
}

//Return the magnitude of an input vector
float Magnitude(t_3vector in)
{
	return magic_sqrt(in.x*in.x + in.y*in.y + in.z*in.z);
}

void RotateOutline(const tPolygon *outline, tPolygon *rotatedoutline, float cosangle, float sinangle)
{
	for(int i=0;i<outline->points;i++)
	{
		rotatedoutline->vertex[i].x = cosangle * outline->vertex[i].x - sinangle * outline->vertex[i].y;
		rotatedoutline->vertex[i].y = sinangle * outline->vertex[i].x + cosangle * outline->vertex[i].y;
	}
}

void TranslateOutline(const tPolygon *rotatedoutline, tPolygon *predictedoutline, const t_vector *position)
{
	for(int i=0;i<rotatedoutline->points;i++)
	{
		predictedoutline->vertex[i].x = rotatedoutline->vertex[i].x + position->x;
		predictedoutline->vertex[i].y = rotatedoutline->vertex[i].y + position->y;
	}
}

t_point GetRadarPosition(t_vector object, int player)
{
	t_point position;
	PosToScreen(&object,&position,player);

	position.x -= ship[player].scrposition.x;
	position.y -= ship[player].scrposition.y;

	position.x = lrintf(position.x*ship[player].radarrange) + (RADARWIDTH>>1);
	position.y = lrintf(position.y*ship[player].radarrange) + (RADARHEIGHT>>1);

	return position;
}

//Finds whether a point is inside a polygon
bool PointInPolyOld(t_vector hitPos, const tPolygon *poly)
{
	int quad, nextquad, i, delta, total=0, next=0, current=0;
	float temp1, temp2, temp3;

	//Which quadrant is it in?
	quad = ((hitPos.x<poly->vertex[next].x)?((hitPos.y<poly->vertex[next].y)?1:4):((hitPos.y<poly->vertex[next].y)?2:3));

	for(i=1;i<=poly->points;i++, current++)
	{
		if((++next)==poly->points) next = 0;	//Make 'next' refer to next point
		//Start at first point, work out which quadrant
		nextquad = ((hitPos.x<poly->vertex[next].x)?((hitPos.y<poly->vertex[next].y)?1:4):((hitPos.y<poly->vertex[next].y)?2:3));
		delta = nextquad-quad;

		switch (delta)
		{
		case 2:		// If we crossed the middle, figure out if it was clockwise or counter
		case -2:	// Use the x position at the hit point to determine which way around
			temp1 = (poly->vertex[next].y - hitPos.y) * (poly->vertex[current].x - poly->vertex[next].x);	//-c.dx
			temp2 = poly->vertex[current].y - poly->vertex[next].y;		//dy
			temp3 = temp1/temp2;
			if ( (poly->vertex[next].x - temp3) > hitPos.x) delta = -(delta);
			break;
	    case 3:		//Moving 3 quads is like moving back 1
			delta = -1; 
			break;
	    case -3:	//Moving back 3 is like moving forward 1
			delta = 1;
			break;
	    }
		//Add the delta
		total += delta;
		quad = nextquad;
	}
	return (total==4 || total==-4)?true:false;
}

bool PointInPoly(t_vector hitPos, const tPolygon *poly)
{
	// Local Variables 
	short first = 0, next = 0;
	t_vector pnt1, pnt2;
	bool inside = false;	//Initial test condition
	bool flag1, flag2;

	pnt1 = poly->vertex[0];
	flag1 = ( hitPos.y >= pnt1.y );	//Is the first vertex over or under the line?

	/* Loop through the vertices in a sector */
	do
	{
		++next;	// Check the next vertex			
		if(next == poly->points) next = 0;
		pnt2 = poly->vertex[next];
		flag2 = ( hitPos.y >= pnt2.y );	// Is it over or under
		if (flag1 != flag2)		//Make sure the edge actually crosses the test x axis
		{	// Calculate whether the segment actually crosses the x test axis
			// A trick from graphic gems IV to get rid of the x intercept divide
			if (((pnt2.y - hitPos.y) * (pnt1.x - pnt2.x) >=	(pnt2.x - hitPos.x) * (pnt1.y - pnt2.y)) == flag2 )
			inside = !inside; // If it crosses toggle the inside flag (odd is in, even out)
			flag1 = flag2;
		}
		pnt1 = pnt2;	//Reset for next step
	} while(next);
	return inside;
}

//This function is one of the most-called in the entire game, for levels with lots of boundaries
bool PolyCollisionDetect(const tPolygon *poly1, const tPolygon *poly2)
{
	int i;
	for(i=0;i<poly1->points;++i)
		if(PointInPoly(poly1->vertex[i],poly2)) return true;	//Finds whether a point of poly1 is inside poly2
	for(i=0;i<poly2->points;++i)
		if(PointInPoly(poly2->vertex[i],poly1)) return true;	//Finds whether a point of poly2 is inside poly1
	return false;
}

//This version uses line intersections instead of points
bool FullPolyCollisionDetect(const tPolygon *poly1, const tPolygon *poly2)
{
	int i, next = 1;
	tPolygon line(2);

	for(i=0;i<poly1->points;i++,next++)
	{
		if(next==poly1->points) next = 0;
		line.vertex[0] = poly1->vertex[i];
		line.vertex[1] = poly1->vertex[next];
		if(LinePolygonIntersection(&line, poly2)) return true;	//Finds whether a point of poly1 is inside poly2
	}

	return false;
}

//Creates a rectangular boundary for an object without an outline of its own
void CreatePolygonOutline(tPolygon *outline, float x, float y)
{
	x*=0.5;
	y*=0.5;
	outline->SetPoints(4);
	outline->vertex[0].x = -x;
	outline->vertex[0].y = -y;
	outline->vertex[1].x = x;
	outline->vertex[1].y = -y;
	outline->vertex[2].x = x;
	outline->vertex[2].y = y;
	outline->vertex[3].x = -x;
	outline->vertex[3].y = y;
}

float FindRadius(const tPolygon *outline)
{
	float distance = 0, furthestdistance = 0;
	for(int i=0;i<outline->points;i++)
	{
		distance = outline->vertex[i].x*outline->vertex[i].x + outline->vertex[i].y*outline->vertex[i].y;
		if(distance>furthestdistance) furthestdistance = distance;
	}
	return furthestdistance + 4.0f;
}

//Will return true if two line segments p1p2 and p3p4 intersect. If so, intersection point will be in *intersect
bool LinesIntersectionPoint(const t_vector *p1, const t_vector *p2, const t_vector *p3, const t_vector *p4, t_vector *intersect)
{
	float dx1, dx2, dy1, dy2, mx, my, ua, ub, D;
	dx1 = p2->x - p1->x;
	dx2 = p4->x - p3->x;
	dy1 = p2->y - p1->y;
	dy2 = p4->y - p3->y;

	D = dy2*dx1 - dx2*dy1;
	if(D==0) return false;
	D = 1.0f/D;

	mx = p1->x - p3->x;
	my = p1->y - p3->y;

	ua = (dx2*my - dy2*mx)*D;
	if(ua<0 || ua>1.0f) return false;

	ub = (dx1*my - dy1*mx)*D;
	if(ub<0 || ub>1.0f) return false;

	//Intersection is within both line segments - i.e. line segments intersect
	intersect->x = p1->x + ua*dx1;
	intersect->y = p1->y + ua*dy1;
	return true;
}

bool NearestPoint(const t_vector *P1, const t_vector *P2, const t_vector *P3, t_vector *point)
{
	float u, dx, dy, den;

	dx = P2->x - P1->x;		//Work out differences
	dy = P2->y - P1->y;

	den = dx*dx + dy*dy;		//Denominator is square of distance
	if(den==0) return false;

	u = ((P3->x-P1->x)*dx + (P3->y-P1->y)*dy)/den;		//Work out u

	if(u<0 || u>1) return false;

	point->x = P1->x + u*dx;		//Position of point
	point->y = P1->y + u*dy;

	return true;
}

float PointLineSegmentDistance(const t_vector *P1, const t_vector *P2, const t_vector *P3, t_vector *point)
{
	float u, dx, dy, den;

	den = 

	dx = P2->x - P1->x;		//Work out differences
	dy = P2->y - P1->y;

	den = dx*dx + dy*dy;		//Denominator is square of distance
	if(den==0) return false;

	u = ((P3->x-P1->x)*dx + (P3->y-P1->y)*dy)/den;		//Work out u

	if(u<0 || u>1.0f) return 1.0e12f;

	point->x = P1->x + u*dx;		//Position of point
	point->y = P1->y + u*dy;

	dx = point->x - P3->x;
	dy = point->y - P3->y;

	return magic_sqrt(dx*dx + dy*dy);
}

float PointLineSegmentDistanceSq(const t_vector *P1, const t_vector *P2, t_vector *P3, t_vector *point)
{
	float u, dx, dy, den;

	dx = P2->x - P1->x;		//Work out differences
	dy = P2->y - P1->y;

	den = dx*dx + dy*dy;		//Denominator is square of distance
	if(den==0) return 1.0e12f;

	u = ( (P3->x - P1->x)*dx + (P3->y - P1->y)*dy )/den;		//Work out u

	if(u<0 || u>1.0f) return 1.0e12f;

	point->x = P1->x + u*dx;		//Position of point
	point->y = P1->y + u*dy;

	dx = point->x - P3->x;
	dy = point->y - P3->y;

	return dx*dx + dy*dy;
}

void WrapToLevel(t_vector *position)
{
	if(position->x>=level.fwidth) position->x -= level.fwidth;
	else if(position->x<0) position->x += level.fwidth;
	if(position->y>=level.fheight) position->y -= level.fheight;
	else if(position->y<0) position->y += level.fheight;
}

//Find the point in either polygon that is nearest to a line of the other polygon
float PointThatsNearest(const tPolygon *poly1, const tPolygon *poly2, tPolygon *output, int *polygon, t_vector *point)
{
	int i, j, next;
	float distance, mindistance = 1.0e11f;
	t_vector storepoint;

	*polygon = 0;

	//Check every point in polygon 1 with every line in polygon 2
	for(j=0;j<poly1->points;j++)	//Loop over POINTS
	{
		next = 1;
		for(i=0;i<poly2->points;i++,next++)		//Loop over LINES
		{
			if(next==poly2->points) next = 0;
			//Check a LINE from poly2 with a POINT from poly1
			distance = PointLineSegmentDistanceSq(&(poly2->vertex[i]), &(poly2->vertex[next]), &(poly1->vertex[j]), &storepoint );
			if(distance<mindistance)
			{
				mindistance = distance;
				output->vertex[0] = poly2->vertex[i];
				output->vertex[1] = poly2->vertex[next];
				*point = storepoint;
			}
		}
	}
	//Check every point in polygon 2 with every line in polygon 1
	for(j=0;j<poly2->points;j++)	//Loop over POINTS
	{
		next = 1;
		for(i=0;i<poly1->points;i++,next++)		//Loop over LINES
		{
			if(next==poly1->points) next = 0;
			//Check a LINE from poly1 with a POINT from poly2
			distance = PointLineSegmentDistanceSq(&(poly1->vertex[i]), &(poly1->vertex[next]), &(poly2->vertex[j]), &storepoint );
			if(distance<mindistance)
			{
				mindistance = distance;
				output->vertex[0] = poly1->vertex[i];
				output->vertex[1] = poly1->vertex[next];
				*point = storepoint;
				*polygon = 1;
			}
		}
	}
	return mindistance;
}

void ResolveVector(const tPolygon *line, t_vector *velocity)
{
	t_vector linevec, vel = *velocity, normal;
	float anglex, velmag, linemag;

	linevec.x = line->vertex[0].x - line->vertex[1].x;
	linevec.y = line->vertex[0].y - line->vertex[1].y;

	normal = CalculateNormal(linevec);

	velmag = Normalise(&vel);		//Returns INVERSE magnitudes of these vectors and normalises
	linemag = Normalise(&linevec);

	//Get angle between line and vector

	anglex = acos(linevec.x/linemag);	//Angle of colliding line with x axis
	//angley = asin(linevec.x/linemag);	//Angle of colliding line with y axis
	//dbout<<anglex<<endl;
	//dot = linevec.x*vel.x + linevec.y*vel.y;	//Here we get the angle between the line and the velocity vector using
	//angle = acos(dot*velmag*linemag);			//angle = acos( a.b / |a||b| )

	//magnitude = cos(angle)/velmag;			//Resolve velocity vector through angle
	velocity->x = 0;//linevec.x * magnitude;
	velocity->y = 0;//linevec.y * magnitude;

	//
	//dbout<<angle<<" "<<dot<<" "<<sin(angle)<<endl;
	//dbout<<angle<<" "<<velocity->x<<" "<<velocity->y<<endl;
}

t_vector CalculateNormal(t_vector invector)
{
	t_vector normal;
	normal.x = -invector.y;
	normal.y = invector.x;
	Normalise(&normal);
	return normal;
}

//'Bounce' a vector off a line
t_vector BounceVector(const tPolygon *line, t_vector *velocity)
{
	t_vector linevec, vel = *velocity, normal;
	float dot, velmag;

	linevec.x = line->vertex[1].x - line->vertex[0].x;		//dx
	linevec.y = line->vertex[1].y - line->vertex[0].y;		//dy

	normal = CalculateNormal(linevec);

	velmag = Normalise(&vel);		//Returns INVERSE magnitudes of these vectors and normalises

	dot = vel.x*normal.x + vel.y*normal.y;
	velocity->x = vel.x - 2.0f*normal.x*dot;
	velocity->y = vel.y - 2.0f*normal.y*dot;

	return normal;
}

void UnitVectorTowards(t_vector pos, t_vector dest, t_vector *unitvector, float *dist)
{
	float x, y, para, nconst;
	x = dest.x - pos.x;
	y = dest.y - pos.y;
	para = x*x + y*y;
	nconst = magic_inv_sqrt(para);
	unitvector->x = x*nconst;
	unitvector->y = y*nconst;
	*dist = para*nconst;
	return;
}

void UnitVectorAway(t_vector pos, t_vector dest, t_vector *unitvector, float *dist)
{
	float x, y, para, nconst;
	x = dest.x - pos.x;
	y = dest.y - pos.y;
	para = x*x + y*y;
	nconst = magic_inv_sqrt(para);
	unitvector->x = -x*nconst;
	unitvector->y = -y*nconst;
	*dist = para*nconst;
	return;
}

float DistanceSquared(t_vector one, t_vector two)
{
	float dx = one.x - two.x;
	float dy = one.y - two.y;

	if( dx > level.hwidth ) dx -= level.fwidth;
	else if( dx < level.mhwidth ) dx += level.fwidth;

	if( dy > level.hheight ) dy -= level.fheight;
	else if( dy < level.mhheight ) dy += level.fheight;

	return dx*dx + dy*dy;
}

float Distance(t_vector one, t_vector two)
{
	float dx = one.x - two.x;
	float dy = one.y - two.y;

	if( dx > level.hwidth ) dx -= level.fwidth;
	else if( dx < level.mhwidth ) dx += level.fwidth;

	if( dy > level.hheight ) dy -= level.fheight;
	else if( dy < level.mhheight ) dy += level.fheight;

	return magic_sqrt(dx*dx + dy*dy);
}

bool LinePolygonIntersection(const tPolygon *line, const tPolygon *poly)
{
	int i, next = 1;
	t_vector point;
	if(line->points!=2) return false;
	for(i=0;i<poly->points;i++,next++)
	{
		if(next==poly->points) next = 0;
		if(LinesIntersectionPoint(&line->vertex[0], &line->vertex[1], &poly->vertex[i], &poly->vertex[next], &point)) return true;
	}
	return false;
}

t_vector RotateVectorQuick(t_vector vect, float ca, float sa)
{
	t_vector out;
	out.x = (vect.x*ca) - (vect.y*sa);
	out.y = (vect.x*sa) + (vect.y*ca);
	return out;
}

t_vector RotateVector(t_vector vect, float angle)
{
	t_vector out;
	Fsincos_t sincos = fsincos(angle);
//	float sa = sin(angle);
//	float ca = cos(angle);
	out.x = (vect.x*sincos.cosine) - (vect.y*sincos.sine);
	out.y = (vect.x*sincos.sine) + (vect.y*sincos.cosine);
	return out;
}

bool OnScreen(GameObject *object, int player)
{	t_vector position, rposition;
	t_point junk;
	float x, y;

	//No good for wrapping levels
//	if(!PointInPoly(object->position,&ship[player].screen)) return false;

	if(PosToScreen(&object->position,&(object->scrposition[player]),player)) return true;

	if(CHECK(object->state,POINTDETECTION)) return false;

	x = (float)object->graphics->framewidth*0.5f;
	y = (float)object->graphics->frameheight*0.5f;

	//Check bottom right
	position.x = x;
	position.y = y;
	rposition = RotateVectorQuick(position, object->cosangle, object->sinangle);
	rposition.x += object->position.x;
	rposition.y += object->position.y;
	if(PosToScreen(&rposition,&junk,player)) return true;

	//Check top right
	position.x = x;
	position.y = -y;
	rposition = RotateVectorQuick(position, object->cosangle, object->sinangle);
	rposition.x += object->position.x;
	rposition.y += object->position.y;
	if(PosToScreen(&rposition,&junk,player)) return true;

	//Check top left
	position.x = -x;
	position.y = -y;
	rposition = RotateVectorQuick(position, object->cosangle, object->sinangle);
	rposition.x += object->position.x;
	rposition.y += object->position.y;
	if(PosToScreen(&rposition,&junk,player)) return true;

	//Check bottom left
	position.x = -x;
	position.y = y;
	rposition = RotateVectorQuick(position, object->cosangle, object->sinangle);
	rposition.x += object->position.x;
	rposition.y += object->position.y;
	if(PosToScreen(&rposition,&junk,player)) return true;

	return false;
}

bool OnScreenFade(FadeObject *object, int player)
{
	if(PosToScreen(&object->position,&(object->scrposition[player]),player)) return true;
	return false;
}

bool TileOnScreen(t_vector *position, t_point *scrposition, int player)
{	t_point junk;

	//Check centre - most tiles will return from here straight away
	if(PosToScreen(position,scrposition,player)) return true;
	//Check bottom right
	position->x += HALFTILESIZE;
	position->y += HALFTILESIZE;
	if(PosToScreen(position,&junk,player)) return true;
	//Check bottom left
	position->x -= TILESIZE;
	if(PosToScreen(position,&junk,player)) return true;
	//Check top left
	position->y -= TILESIZE;
	if(PosToScreen(position,&junk,player)) return true;
	//Check top right
	position->x += TILESIZE;
	if(PosToScreen(position,&junk,player)) return true;
	return false;
}


int WhichTile(int x, int y)
{
	//Shift - to create infinitely wrapping levels
	if(x>=level.width) x -= level.width;		//Shift to the left domain
	else if(x<0) x += level.width;			//Shift to the right domain

	if(y>=level.height) y -= level.height;	//Shift to the left domain
	else if(y<0) y += level.height;			//Shift to the right domain

	return level.tilemap[y*level.width + x];
}

//Bascally tests two outer lines at width of object, parallel to desired course, for boundary collisions
bool TestPathIntersection(t_vector point1, t_vector point2, int width)
{
	tPolygon line(2);
	t_vector normd;
//	line.vertex[0] = point1;
//	line.vertex[1] = point2;
//	if(LineBoundariesCollision(line)) return true;		//Collision detected, path not clear
	normd.x = point2.x - point1.x;		//Find vector from 1 to 2
	normd.y = point2.y - point1.y;
	Normalise(&normd);		//Normalise to unit vector
	normd.y *= width;		//Multiply to width of object
	normd.x *= width;
	// PI/2 rotation (anticlockwise)
	line.vertex[0].x = point1.x - normd.y;
	line.vertex[0].y = point1.y + normd.x;
	line.vertex[1].x = point2.x - normd.y;
	line.vertex[1].y = point2.y + normd.x;
	if(LineBoundariesCollision(line)) return true;		//Collision detected, path not clear
	//-PI/2 rotation (clockwise)
	line.vertex[0].x = point1.x + normd.y;
	line.vertex[0].y = point1.y - normd.x;
	line.vertex[1].x = point2.x + normd.y;
	line.vertex[1].y = point2.y - normd.x;
	if(LineBoundariesCollision(line)) return true;		//Collision detected, path not clear
	return false;		//No collision, path clear
}

bool TestPlayerSighting(t_vector enemyposition, t_vector *playerposition, int player, int type)
{
	int playerwidth = ship[player].graphics.framewidth>>1;		//Modifying this will make the turrets shoot at corners wher player is hiding
	tPolygon line(2);
	t_vector normd;
	line.vertex[0] = enemyposition;
	line.vertex[1] = *playerposition;
	if(!LineBoundariesCollision(line)) return false;		//Collision not detected, path clear

	if(type==ENEMY) return true;		//No good, can't see player

	normd.x = playerposition->x - enemyposition.x;
	normd.y = playerposition->y - enemyposition.y;
	Normalise(&normd);

	// PI/2 rotation
	line.vertex[1].x = playerposition->x - normd.y*playerwidth;
	line.vertex[1].y = playerposition->y + normd.x*playerwidth;
	if(!LineBoundariesCollision(line))
	{
		*playerposition = line.vertex[1];
		return false;		//Collision not detected, path clear, return sighted point
	}

	//-PI/2 rotation
	line.vertex[1].x = playerposition->x + normd.y*playerwidth;
	line.vertex[1].y = playerposition->y - normd.x*playerwidth;
	if(!LineBoundariesCollision(line))
	{
		*playerposition = line.vertex[1];
		return false;		//Collision not detected, path clear, return sighted point
	}

	return true;		//No good, can't see player
}


//Will return the angle between a vector at origin and a target point
float AngleToPoint(t_vector origin, t_vector target, t_vector heading)
{
	float x = target.x - origin.x;
	float y = target.y - origin.y;
	float xl = heading.x;		//x component of ship's angle - (optimisation: same as uvf??)
	float yl = heading.y;		//y component of ship's angle
	float dist = magic_inv_sqrt(x*x+y*y);					//distance to selected point
	float arg = (-xl*x + -yl*y) * dist;
	float crossp, angle;

	if(arg>1.0 || arg<-1.0) return 0;		//Sometimes, due to rounding error, this may come to be greater than one, causing a NaN

	crossp = xl*y - yl*x;
	angle = acos(arg);
	if(crossp>0) angle = -angle;
	return angle;
}
