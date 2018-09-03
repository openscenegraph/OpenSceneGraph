package osg.AndroidExample;

import android.app.Dialog;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Shader;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;

public class ColorPickerDialog extends Dialog{
	
	public interface OnColorChangeListener{
		void colorChange(int color);
	}
	
	private OnColorChangeListener tListener;
	private int tInitialColor;
	
	private static class ColorPickerView extends View{
		
		private Paint 			tPaint;
		private float 			tCurrentHue = 0;
		private int 			tCurrentX = 0;
		private int 			tCurrentY = 0;
		private int 			tCurrentColor;
		private final int[] 	tHueGradientColors = new int [258];
		private int []			tGradientColors = new int[65536]; //256X256 colors
		private OnColorChangeListener tListener;
		private boolean			tQSelected = false;	
		
		public ColorPickerView(Context context, OnColorChangeListener listener, int color) {
			super(context);
			// TODO Auto-generated constructor stub
			tListener = listener;
			
			//Get Hue from tCurrentColor and update the Gradient of Color
			float[] newHSV = new float[3];
			Color.colorToHSV(color, newHSV);
			tCurrentHue = newHSV[0];
			updateGradientColors();
			tCurrentColor = color;
			
			//Initialize of colors in Hue slider
			
			int index = 0;
			for(float i=0; i<256; i += 256/42 , index++){
				tHueGradientColors[index] = Color.rgb(255, 0, (int)i);
			}
			for(float i=0; i<256; i += 256/42 , index++){
				tHueGradientColors[index] = Color.rgb(255-(int)i, 0, 255);
			}
			for(float i=0; i<256; i += 256/42 , index++){
				tHueGradientColors[index] = Color.rgb(0, (int) i, 255);
			}
			for(float i=0; i<256; i += 256/42 , index++){
				tHueGradientColors[index] = Color.rgb(0, 255, 255-(int)i);
			}
			for(float i=0; i<256; i += 256/42 , index++){
				tHueGradientColors[index] = Color.rgb((int)i, 255, 0);
			}
			for(float i=0; i<256; i += 256/42 , index++){
				tHueGradientColors[index] = Color.rgb(255, 255-(int)i, 0);
			}
			
			// Paint initialized
			tPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
			tPaint.setTextAlign(Paint.Align.CENTER);
			tPaint.setTextSize(12);
		}
		
		// Get the Color from Hue Bar
		private int getCurrentGradientColor(){
			
			int currentHue = 255 - (int)(tCurrentHue*255/360);
			int index = 0;
			for (float i=0; i<256; i += 256/42, index++){
				if (index == currentHue) return Color.rgb(255, 0, (int) i );
			}
			for (float i=0; i<256; i += 256/42, index++){
				if (index == currentHue) return Color.rgb(255-(int)i, 0, 255 );
			}
			for (float i=0; i<256; i += 256/42, index++){
				if (index == currentHue) return Color.rgb(0, (int) i, 255 );
			}
			for (float i=0; i<256; i += 256/42, index++){
				if (index == currentHue) return Color.rgb(0, 255, 255-(int) i );
			}
			for (float i=0; i<256; i += 256/42, index++){
				if (index == currentHue) return Color.rgb((int) i, 255, 0 );
			}
			for (float i=0; i<256; i += 256/42, index++){
				if (index == currentHue) return Color.rgb(255, 255-(int) i, 0);
			}
			return Color.RED;
		}
		
		private void updateGradientColors(){
			
			int actualColor = getCurrentGradientColor();
			int index = 0;
			int[] colColors = new int[256];
			for(int y=0; y<256; y++){
				for(int x=0; x<256; x++ , index++){
					if(y==0){
						tGradientColors[index] = Color.rgb(255-(255-Color.red(actualColor))*x/255, 255-(255-Color.green(actualColor))*x/255, 255-(255-Color.blue(actualColor))*x/255);
						colColors[x] = tGradientColors[index];
					}
					else{
						tGradientColors[index] = Color.rgb((255-y)*Color.red(colColors[x])/255, (255-y)*Color.green(colColors[x])/255, (255-y)*Color.blue(colColors[x])/255);
					}
				}
			}
		}
		
		@Override
		protected void onDraw(Canvas canvas){
			int translatedHue = 255 - (int)(tCurrentHue*255/360);
			//Display HUE with bar lines
			
			for(int x=0; x<256; x++){
				//We display the color or a big white bar
				if(translatedHue != x){
					tPaint.setColor(tHueGradientColors[x]);
					tPaint.setStrokeWidth(1);
				}
				else{
					tPaint.setColor(Color.WHITE);
					tPaint.setStrokeWidth(3);
				}
				canvas.drawLine(x+10, 0, x+10, 40, tPaint);
			}
			
			// Display Gradient Box
			for(int x=0; x<256;x++){
				int[] colors = new int[2];
				colors[0] = tGradientColors[x];
				colors[1] = Color.BLACK;
				Shader shader = new LinearGradient(0,50,0,306,colors,null, Shader.TileMode.REPEAT);
				tPaint.setShader(shader);
				canvas.drawLine(x+10, 50, x+10, 306, tPaint);
			}
			tPaint.setShader(null);
			
			//Display the circle around the currently selected color in the main field
			if(tCurrentX !=0 && tCurrentY != 0){
				tPaint.setStyle(Paint.Style.STROKE);
				tPaint.setColor(Color.BLACK);
				canvas.drawCircle(tCurrentX, tCurrentY, 10, tPaint);
			}
			
			//Draw a button
			
			tPaint.setStyle(Paint.Style.FILL);
			if(tQSelected){
				tPaint.setColor(Color.WHITE);
				canvas.drawCircle(138, 336, 30, tPaint);
			}
			tPaint.setColor(tCurrentColor);
			canvas.drawCircle(138, 336, 20, tPaint);
			
		}
		
		@Override
		protected void onMeasure(int width,int height){
			setMeasuredDimension(276, 366);
		}
		
		@Override
		public boolean onTouchEvent(MotionEvent event){
			if(event.getAction() != MotionEvent.ACTION_DOWN && event.getAction() != MotionEvent.ACTION_MOVE && event.getAction() != MotionEvent.ACTION_UP) return true;
			float x = event.getX();
			float y = event.getY();
			
			tQSelected=false;
			
			// if in Hue Bar
			if(x >10 && x <266 && y>0 && y<40){
				//Update gradient
				tCurrentHue = (255-x)*360/255;
				updateGradientColors();
				
				//Update current Selected Color
				int nX = tCurrentX-10;
				int nY = tCurrentY-60;
				int index = 256 * (nY-1)+nX;
				
				if(index>0 && index < tGradientColors.length)
					tCurrentColor = tGradientColors[256*(nY-1)+nX];
				
				invalidate(); //By invalidating we are forcing a redraw;
			}
			
			// If Main gradient
			
			if ( x >10 && x< 266 && y>50 && y <306){
				tCurrentX = (int) x;
				tCurrentY = (int) y;
				int nX = tCurrentX - 10;
				int nY = tCurrentY - 60;
				int index = 256*(nY-1)+nX;
				if (index >0 && index < tGradientColors.length){
					tCurrentColor = tGradientColors[index];
					
					invalidate(); //By invalidating we are forcing a redraw;
				}
			}
			if( x>118 && x<158 && y > 316 && y <356){
				if(event.getAction() == MotionEvent.ACTION_DOWN || event.getAction() == MotionEvent.ACTION_MOVE){
					tQSelected=true;
				}
				if(event.getAction() == MotionEvent.ACTION_UP){
					tQSelected=false;
					tListener.colorChange(tCurrentColor);
				}
				invalidate();
			}
			
			
			
			return true;
		}
		
		
	}
	
	public ColorPickerDialog(Context context, OnColorChangeListener listener, int initialColor){
		super(context);
		
		tListener = listener;
		tInitialColor = initialColor;
	}
	
	@Override
	protected void onCreate( Bundle savedInstanceState){
		super.onCreate(savedInstanceState);
		OnColorChangeListener l = new OnColorChangeListener(){
			public void colorChange(int color){
				tListener.colorChange(color);
				dismiss();
			}
		};
		
		setContentView(new ColorPickerView(getContext(),l,tInitialColor));
	}

}
