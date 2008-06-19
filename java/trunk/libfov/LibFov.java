package libfov;

import java.util.Hashtable;
import libfov.Settings.DirectionType;
import libfov.Settings.OpaqueApplyType;
import libfov.Settings.ShapeType;


class fov_private_data_type
{
    Settings settings;
    IFovFunctions functions;
    int source_x;
    int source_y;
    int radius;
}

/**
 * This is a Java port of C library which implements a course-grained 
 * lighting algorithm suitable for tile-based games such as roguelikes.
 * 
 * For the original library go to http://sourceforge.net/projects/libfov
 * 
 */
public class LibFov 
{

/*
  
         Shared
        edge by
Shared     1 & 2      Shared
edge by\      |      /edge by
1 & 8   \     |     / 2 & 3
         \1111|2222/
         8\111|222/3
      	 88\11|22/33
      	 888\1|2/333
Shared   8888\|/3333  Shared
edge by-------@-------edge by
7 & 8    7777/|\4444  3 & 4
         777/6|5\444
         77/66|55\44
         7/666|555\4
         /6666|5555\
Shared  /     |     \ Shared
edge by/      |      \edge by
6 & 7      Shared     4 & 5
          edge by 
            5 & 6
*/

	
	private static Hashtable<DirectionType, int[]> DIRECTIONS = new Hashtable<DirectionType, int[]>();
	static
	{
		//dx is positive x, dy is negative y => sector 3 (EAST)
		DIRECTIONS.put(DirectionType.FOV_EAST, 		new int[]{ 1, 0, 0,-1});
		//dx is positive x, dy is positive y => sector 4 (SOUTHEAST)
		DIRECTIONS.put(DirectionType.FOV_SOUTHEAST, new int[]{ 1, 0, 0, 1});
		//dx is positive y, dy is positive x => sector 5 (SOUTH)
		DIRECTIONS.put(DirectionType.FOV_SOUTH,		new int[]{ 0, 1, 1, 0});
		//dx is negative y, dy is positive x => sector 6 (SOUTHWEST)
		DIRECTIONS.put(DirectionType.FOV_SOUTHWEST,	new int[]{ 0,-1, 1, 0});
		//dx is negative x, dy is positive y => sector 7 (WEST)
		DIRECTIONS.put(DirectionType.FOV_WEST,		new int[]{-1, 0, 0, 1});
		//dx is negative x, dy is negative y => sector 8 (NORTHWEST)
		DIRECTIONS.put(DirectionType.FOV_NORTHWEST,	new int[]{-1, 0, 0,-1});
		//dx is negative y, dy is negative x => sector 1 (NORTH)
		DIRECTIONS.put(DirectionType.FOV_NORTH,		new int[]{ 0,-1,-1, 0});
		//dx is positive x, dy is negative y => sector 3 (NORTHEAST)
		DIRECTIONS.put(DirectionType.FOV_NORTHEAST,	new int[]{ 0, 1,-1, 0});
	}
	
	/**
	 * Limit x to the range [a, b].
	 */
	private static float betweenf(float x, float a, float b) 
	{
		return Math.max(a,Math.min(x,b));		
	}
	
	/* Circular FOV --------------------------------------------------- */

	private static int[] precalculate_heights(int maxdist) 
	{
		int result[] = new int[maxdist + 2];

		for (int i = 0; i <= maxdist; ++i) 
		{
			result[i] = (int) Math.sqrt((maxdist * maxdist - i * i));
		}
		result[maxdist + 1] = 0;

		return result;
	}

	private static int height(Settings settings, int x, int maxdist) 
	{
		int[][] newHeights;
		if (maxdist > settings.numheights) 
		{
			newHeights = new int[maxdist][];
			settings.heights = newHeights;
			settings.numheights = maxdist;
		}
		if (settings.heights != null) 
		{
			if (settings.heights[maxdist - 1] == null) 
			{
				settings.heights[maxdist - 1] = precalculate_heights(maxdist);
			}
			if (settings.heights[maxdist - 1] != null) 
			{
				return settings.heights[maxdist - 1][Math.abs(x)];
			}
		}
		return 0;
	}
	
	private static float fov_slope(float dx, float dy) 
	{
	    if (dx <= 0.0 || dx >= 0.0) 
	    {
	        return dy/dx;
	    } else 
	    {
	        return 0.0f;
	    }
	}	
	
	private static void fov_octant (fov_private_data_type data, int dx, float start_slope, float end_slope, int[] mults)
	{
        int x, y, dy, yStart, yEnd;
        int h;
        int prev_blocked = -1;
        float end_slope_next;
        Settings settings = data.settings;
        IFovFunctions functions = data.functions;
        
        //Checking for boundary cases
        if (dx == 0) 
        {                                                                          
            fov_octant(data, dx+1, start_slope, end_slope,mults); 
            return;                                                                             
        } else if (dx > data.radius) 
        {
            return;
        }         
        
        //Calculating starting and ending y offsets
        yStart = (int)(0.5f + dx*start_slope);                                            
        yEnd = (int)(0.5f + dx*end_slope);                                              

            
        //Checking shape
        if (settings.shape == ShapeType.FOV_SHAPE_CIRCLE_PRECALCULATE)
        {
        	h = height(settings, dx, data.radius);
        }
        else if (settings.shape == ShapeType.FOV_SHAPE_CIRCLE)
        {
        	h = (int)Math.sqrt(data.radius*data.radius - dx*dx);        	
        }
        else if (settings.shape == ShapeType.FOV_SHAPE_OCTAGON)
        {
        	h = (data.radius - dx)<<1;
        }
        else
        {
        	h = data.radius;
        }
        
        if (yEnd > h) 
        {
            if (h == 0)
            {
                return;
            }
            yEnd = h;            
        }  

        //Main iteration over the row/column
        for (dy = yStart; dy <= yEnd; ++dy) 
        {                                 
        	//calculating current x and y coordinates
        	x = data.source_x + dx*mults[0]+dy*mults[1];
        	y = data.source_y + dx*mults[2]+dy*mults[3];                                                                
                                                                             
            //If tile is opaque
            if (functions.TestOpacity(x, y)) 
            {
            	if (settings.opaqueApply==OpaqueApplyType.FOV_OPAQUE_APPLY)
            	{
            		functions.ApplyLighting( x, y);
            	}
                                                                                              
                if (prev_blocked == 0) 
                {                                                        
                    end_slope_next = fov_slope(dx + 0.5f, dy - 0.5f);             
                    fov_octant(data, dx+1, start_slope, end_slope_next,mults);           
                }                                                                               
                prev_blocked = 1;                                                               
            } 
            else 
            {          
            	functions.ApplyLighting(x, y);
                if (prev_blocked == 1) 
                {                                                        
                    start_slope = fov_slope(dx - 0.5f, dy - 0.5f);                
                }                                                                               
                prev_blocked = 0;                                                               
            }                                                                                   
        }                                                                                       
                                                                                                
        if (prev_blocked == 0) 
        {                                                                
            fov_octant(data, dx+1, start_slope, end_slope,mults);                        
        }              
	}


	/**
	 * Calculate a full circle field of view from a source at (x,y).
	 *
	 * @param settings data structure containing settings.
	 * @param functions reference to the object that will test opacity and apply lighting 
	 * @param source_x x-axis coordinate from which to start.
	 * @param source_y y-axis coordinate from which to start.
	 * @param radius Euclidean distance from (x,y) after which to stop.
	 */
	public static synchronized void fov_circle(Settings settings, IFovFunctions functions,
			int source_x, int source_y, int radius) 
	{		
	    fov_private_data_type data = new fov_private_data_type();

	    data.settings = settings;
	    data.functions = functions;
	    data.source_x = source_x;
	    data.source_y = source_y;
	    data.radius = radius;
	    
	    functions.Init();

	    for (int[]i : DIRECTIONS.values())
	    {
	    	fov_octant(data, 1, 0.0f, 1.0f,i);	    	
	    }

	}	

	/**
	 * Calculate a field of view from source at (x,y), pointing
	 * in the given direction and with the given angle. The larger
	 * the angle, the wider, "less focused" the beam. Each side of the
	 * line pointing in the direction from the source will be half the
	 * angle given such that the angle specified will be represented on
	 * the raster.
	 *
	 * @param settings Pointer to data structure containing settings.
	 * @param functions reference to the object that will test opacity and apply lighting
	 * @param source_x x-axis coordinate from which to start.
	 * @param source_y y-axis coordinate from which to start.
	 * @param radius Euclidean distance from (x,y) after which to stop.
	 * @param direction One of eight directions the beam of light can point.
	 * @param angle The angle at the base of the beam of light, in degrees.
	 */
	public static synchronized void fov_beam(Settings settings, IFovFunctions functions,
			int source_x, int source_y, int radius, DirectionType direction,
			float angle) 
	{
	    fov_private_data_type data = new fov_private_data_type();
	    float start_slope, end_slope, a;

	    data.settings = settings;
	    data.functions = functions;
	    data.source_x = source_x;
	    data.source_y = source_y;
	    data.radius = radius;
	    
	    if (angle <= 0.0f) 
	    {
	        return;
	    } else if (angle >= 360.0f) 
	    {
	    	fov_circle(settings,functions,source_x,source_y,radius);
	    	return;
	    }
	    
	    functions.Init();
	    
	    /* Calculate the angle as a percentage of 45 degrees, halved (for
	     * each side of the centre of the beam). e.g. angle = 180.0f means
	     * half the beam is 90.0 which is 2x45, so the result is 2.0.
	     */
	    a = angle/90.0f;

	    boolean diagonal = direction.isDiagonal();
	    start_slope = diagonal? betweenf(1.0f - a, 0.0f, 1.0f): 0.0f;
	    end_slope 	= diagonal? 1.0f: betweenf(a, 0.0f, 1.0f);
	    DirectionType prev = direction;
	    DirectionType next = direction.next();
	    
	    fov_octant(data, 1, start_slope, end_slope, DIRECTIONS.get(prev));
	    fov_octant(data, 1, start_slope, end_slope, DIRECTIONS.get(next));
	    if (a>1.0f)
	    {
	    	start_slope = diagonal? 0.0f: betweenf(2.0f-a, 0.0f, 1.0f);
	    	end_slope 	= diagonal? betweenf(a - 1.0f, 0.0f, 1.0f): 1.0f;
	    	prev = prev.previous();
		    next = next.next();
	    	fov_octant(data, 1, start_slope, end_slope, DIRECTIONS.get(prev));
	    	fov_octant(data, 1, start_slope, end_slope, DIRECTIONS.get(next));
	    }
	    if (a>2.0f)
	    {
	    	start_slope = diagonal? betweenf(3.0f - a, 0.0f, 1.0f): 0.0f;
	    	end_slope 	= diagonal? 1.0f: betweenf(a-2.0f, 0.0f, 1.0f);
	    	prev = prev.previous();
		    next = next.next();
	    	fov_octant(data,1,start_slope,end_slope,DIRECTIONS.get(prev));
	    	fov_octant(data,1,start_slope,end_slope,DIRECTIONS.get(next));
	    }
	    if (a>3.0f)
	    {
	    	start_slope = diagonal? 0.0f: betweenf(4.0f-a, 0.0f, 1.0f);
	    	end_slope	= diagonal? betweenf(a - 3.0f, 0.0f, 1.0f): 1.0f;
	    	prev = prev.previous();
		    next = next.next();
	    	fov_octant(data, 1, start_slope, end_slope, DIRECTIONS.get(prev));
	    	fov_octant(data, 1, start_slope, end_slope, DIRECTIONS.get(next));
	    }	  
	}
}
