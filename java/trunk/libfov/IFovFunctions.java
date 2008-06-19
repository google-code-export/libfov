package libfov;

/**
 * An interface for functions that test opacity and apply lighting
 * The class that implements this interface must store all the necessary information
 * such as Level, Actor or Object that initiated the call, etc.  
 */
public interface IFovFunctions 
{
	/** Opacity test callback. 
	 * @param x x coordinate of the tile
	 * @param y y coordinate of the tile
	 * @return true if tile is opaque
	 */

	public boolean TestOpacity(int x, int y);

	/** Lighting callback to set lighting on a map tile.
	 * sets visible flag for tile with coordinates x,y in map.
	 * @param x x coordinate of the tile to apply lighting
	 * @param y y coordinate of the tile to apply lighting
	 */
	public void ApplyLighting(int x, int y);
	
	/**
	 * Init function.<br>
	 * This function is called only once every time <code>fov_circle</code>
	 * or <code>fov_beam</code> is called. It lets IFovFunction know that
	 * LibFov initiates shadow cast.
	 */
	public void Init();	
}
