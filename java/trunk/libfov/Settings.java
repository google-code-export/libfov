package libfov;

import java.io.Serializable;

/**
 * A class that contains all the settings for the libFov
 *
 */
public class Settings implements Serializable
{
	private static final long serialVersionUID = 0L;
	
	 /** Shape setting. */
	ShapeType shape						  = ShapeType.FOV_SHAPE_CIRCLE;
	CornerPeekType cornerPeek 			  = CornerPeekType.FOV_CORNER_NOPEEK;
	OpaqueApplyType opaqueApply			  = OpaqueApplyType.FOV_OPAQUE_APPLY;

	/** Pre-calculated data. */
	int heights[][];

	/** Size of pre-calculated data. */
	int numheights = 0;

	/** Eight-way directions. */
	public enum DirectionType
	{
	    FOV_EAST,
	    FOV_SOUTHEAST,
	    FOV_SOUTH,
	    FOV_SOUTHWEST,
	    FOV_WEST,
	    FOV_NORTHWEST,
	    FOV_NORTH,
	    FOV_NORTHEAST;
	    
	    	   
	    /**
	     * Gets next direction
	     * @return neighbour in clockwise direction
	     * i.e. EAST.next() will return SOUTH_EAST
	     */
	    public DirectionType next()
	    {
	    	DirectionType[] types = DirectionType.values();
	    	int index = this.ordinal();
	    	index = (index+1)%types.length;
	    	return types[index];	    	
	    }
	    
	    /** 
	     * Gets previous direction
	     * @return neighbour in counter-clockwise direction
	     * i.e. EAST.previous() will return NORTH_EAST
	     */
	    public DirectionType previous()
	    {
	    	DirectionType[] types = DirectionType.values();
	    	int index = this.ordinal();
	    	index--;
	    	if (index<0) index = types.length-1;
	    	return types[index];
	    }
	    
	    /**
	     * @return true if Direction is one of the following directions
	     * - NORTHEAST
	     * - NORTWEST
	     * - SOUTHEAST
	     * - SOUTHWEST
	     */
	    public boolean isDiagonal()
	    {
	    	int index = this.ordinal();
	    	if (index%2==1) return true;
	    	else return false;
	    }
	}

	/** Values for the shape setting. */
	public enum ShapeType 
	{
		/**
		 * (default) Limit the FOV to a
		 * circle with radius R by precalculating, which consumes more memory
		 * at the rate of 4*(R+2) bytes per R used in calls to fov_circle. 
		 * Each radius is only calculated once so that it can be used again.*/ 
	    FOV_SHAPE_CIRCLE_PRECALCULATE,
	    /** Limit the FOV to an RxR square.*/
	    FOV_SHAPE_SQUARE,
	    /** Limit the FOV to a circle with radius R by
	     *  calculating on-the-fly. */
	    FOV_SHAPE_CIRCLE,
	    /** Limit the FOV to an octagon with maximum radius R.*/
	    FOV_SHAPE_OCTAGON
	}

	/** Values for the corner peek setting. */
	public enum CornerPeekType
	{
		/**
		Renders:
			<pre>
			  ......
			  .....
			  ....
			  ..@#
			  ...#
			</pre>
			 */
	    FOV_CORNER_NOPEEK,
	    /**
	    Renders:
	    	<pre><tt>
	    	  ........
	    	  ........
	    	  ........
	    	  ..@#    
	    	  ...#    
	    	</tt></pre>
	    	*/
	    FOV_CORNER_PEEK
	}

	/** Values for the opaque apply setting. */
	public enum OpaqueApplyType
	{
		/**
		 * (default): Call apply callback on opaque tiles.
		 */
	    FOV_OPAQUE_APPLY,
	    /**
	     * Do not call the apply callback on opaque tiles.
	     */
	    FOV_OPAQUE_NOAPPLY
	}
	
	/**
	 * Set the shape of the field of view.
	 *
	 * @param type type of the FOV shape to use
	 *
	 */
	public void SetShape(ShapeType type)
	{
		shape = type;
	}
	
	/**
	 * <em>NOT YET IMPLEMENTED</em>.
	 *
	 * Set whether sources will peek around corners.
	 *
	 * @param type type of the corner peek algorithm
	 */
	public void SetCornerPeek(CornerPeekType type)
	{
		cornerPeek = type;
	}
	
	/**
	 * Whether to call the apply callback on opaque tiles.
	 *
	 * @param type	
	 */
	public void SetOpaqueApply(OpaqueApplyType type)
	{
		opaqueApply = type;
	}
	
	/**
	 * Set the function object used to test whether a map tile is opaque and 
	 * to apply lighting to a map tile.
	 * 	
	 * @param object The object that contains two functions
	 */
}
