package osg.AndroidExample;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.graphics.Color;
import android.graphics.PointF;
import android.os.Bundle;
import android.util.FloatMath;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ImageButton;

import java.io.File;

public class osgViewer extends Activity implements View.OnTouchListener, View.OnKeyListener, ColorPickerDialog.OnColorChangeListener {
	enum moveTypes { NONE , DRAG, MDRAG, ZOOM ,ACTUALIZE}
	enum navType { PRINCIPAL , SECONDARY }
	enum lightType { ON , OFF }
		
	moveTypes mode=moveTypes.NONE;
	navType navMode = navType.PRINCIPAL;
	lightType lightMode = lightType.ON;
	
	PointF oneFingerOrigin = new PointF(0,0);
	long timeOneFinger=0;
	PointF twoFingerOrigin = new PointF(0,0);
	long timeTwoFinger=0;
	float distanceOrigin;
	
	int backgroundColor;
	
	private static final String TAG = "OSG Activity";
	//Ui elements
    EGLview mView;
    Button uiCenterViewButton;
    Button uiNavigationChangeButton;
    ImageButton uiNavigationLeft;
    ImageButton uiNavigationRight;
    Button uiLightChangeButton;
    
    //Toasts
    Toast msgUiNavPrincipal;
    Toast msgUiNavSecondary;
    Toast msgUiLightOn;
    Toast msgUiLightOff;
    
    //Dialogs
    AlertDialog removeLayerDialog;
    AlertDialog loadLayerAddress;

    //Main Android Activity life cycle
    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.ui_layout_gles);
        //Obtain every Ui element
	        mView= (EGLview) findViewById(R.id.surfaceGLES);
		        mView.setOnTouchListener(this);
		        mView.setOnKeyListener(this);
	        
	        uiCenterViewButton = (Button) findViewById(R.id.uiButtonCenter);
	        	uiCenterViewButton.setOnClickListener(uiListenerCenterView);
	        uiNavigationChangeButton = (Button) findViewById(R.id.uiButtonChangeNavigation);
	        	uiNavigationChangeButton.setOnClickListener(uiListenerChangeNavigation);
	        uiLightChangeButton = (Button) findViewById(R.id.uiButtonLight);
	        	uiLightChangeButton.setOnClickListener(uiListenerChangeLight);
	        	
	    //Creating Toasts
       	msgUiNavPrincipal = Toast.makeText(getApplicationContext(), R.string.uiToastNavPrincipal, Toast.LENGTH_SHORT);
       	msgUiNavSecondary = Toast.makeText(getApplicationContext(), R.string.uiToastNavSecond, Toast.LENGTH_SHORT);
       	msgUiLightOn  = Toast.makeText(getApplicationContext(), R.string.uiToastLightOn, Toast.LENGTH_SHORT);
       	msgUiLightOff  = Toast.makeText(getApplicationContext(), R.string.uiToastLightOff, Toast.LENGTH_SHORT);		
       	
       	//Creating Dialogs
       	
       	LayoutInflater factory = LayoutInflater.from(getApplicationContext());
		final View textEntryView = factory.inflate(R.layout.dialog_text_entry, null);
		AlertDialog.Builder loadLayerDialogBuilder = new AlertDialog.Builder(this);
		loadLayerDialogBuilder.setIcon(R.drawable.web_browser);
		loadLayerDialogBuilder.setTitle(R.string.uiDialogTextAddress);
		loadLayerDialogBuilder.setView(textEntryView);
		loadLayerDialogBuilder.setPositiveButton(R.string.uiDialogOk, new DialogInterface.OnClickListener() {
			
			@Override
			public void onClick(DialogInterface dialog, int which) {
				// TODO Auto-generated method stub
				EditText address;
				address = (EditText) textEntryView.findViewById(R.id.uiEditTextInput);
				osgNativeLib.loadObject(address.getText().toString());
			}
		});
		loadLayerDialogBuilder.setNegativeButton(R.string.uiDialogCancel, new DialogInterface.OnClickListener() {
			
			@Override
			public void onClick(DialogInterface dialog, int which) {
				// TODO Auto-generated method stub
				
			}
		});
		
		loadLayerAddress = loadLayerDialogBuilder.create();
    }
    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
    }
    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
    }
    
    //Main view event processing
    @Override
	public boolean onKey(View v, int keyCode, KeyEvent event) {
		
		return true;
	}
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event){
    	//DO NOTHING this will render useless every menu key except Home
    	int keyChar= event.getUnicodeChar();
    	osgNativeLib.keyboardDown(keyChar);
    	return true;
    }
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event){
    	switch (keyCode){
    	case KeyEvent.KEYCODE_BACK:
    		super.onDestroy();
    		this.finish();
    		break;
    	case KeyEvent.KEYCODE_SEARCH:
    		break;
    	case KeyEvent.KEYCODE_MENU:
    		this.openOptionsMenu();
    		break;
    	default:
    		int keyChar= event.getUnicodeChar();
    		osgNativeLib.keyboardUp(keyChar);    		
    	}
    	
    	return true;
    }
    @Override
    public boolean onTouch(View v, MotionEvent event) {
    	
    	//dumpEvent(event);
    	
    	long time_arrival = event.getEventTime();
    	int n_points = event.getPointerCount();
    	int action = event.getAction() & MotionEvent.ACTION_MASK;
    	
    	switch(n_points){
    	case 1:
    		switch(action){
    		case MotionEvent.ACTION_DOWN:
	    		mode = moveTypes.DRAG;
	    		
	    		osgNativeLib.mouseMoveEvent(event.getX(0), event.getY(0));
	    		if(navMode==navType.PRINCIPAL)
	    			osgNativeLib.mouseButtonPressEvent(event.getX(0), event.getY(0), 2);
	    		else
	    			osgNativeLib.mouseButtonPressEvent(event.getX(0), event.getY(0), 1);
	    		
	    		oneFingerOrigin.x=event.getX(0);
	    		oneFingerOrigin.y=event.getY(0);
    			break;
    		case MotionEvent.ACTION_CANCEL:
    			switch(mode){
    			case DRAG:
    				osgNativeLib.mouseMoveEvent(event.getX(0), event.getY(0));
    				if(navMode==navType.PRINCIPAL)
    					osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 2);
    				else
    					osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 1);
    				break;
    			default :
    				Log.e(TAG,"There has been an anomaly in touch input 1point/action");
    			}
    			mode = moveTypes.NONE;
    			break;
    		case MotionEvent.ACTION_MOVE:
    			
    			osgNativeLib.mouseMoveEvent(event.getX(0), event.getY(0));
    			
    			oneFingerOrigin.x=event.getX(0);
	    		oneFingerOrigin.y=event.getY(0);
	    		
    			break;
    		case MotionEvent.ACTION_UP:
    			switch(mode){
    			case DRAG:
    				if(navMode==navType.PRINCIPAL)
    					osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 2);
    				else
    					osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 1);
    				break;
    			default :
    				Log.e(TAG,"There has been an anomaly in touch input 1 point/action");
    			}
    			mode = moveTypes.NONE;
    			break;
    		default :
    			Log.e(TAG,"1 point Action not captured");	
    		}
    		break;
    	case 2:
    		switch (action){
    		case MotionEvent.ACTION_POINTER_DOWN:
    			//Free previous Action
    			switch(mode){
    			case DRAG:
    				if(navMode==navType.PRINCIPAL)
    					osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 2);
    				else
    					osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 1);
    				break;
    			}
    			mode = moveTypes.ZOOM;
    			distanceOrigin = sqrDistance(event);
    			twoFingerOrigin.x=event.getX(1);
    			twoFingerOrigin.y=event.getY(1);
    			oneFingerOrigin.x=event.getX(0);
	    		oneFingerOrigin.y=event.getY(0);
    			
    			osgNativeLib.mouseMoveEvent(oneFingerOrigin.x,oneFingerOrigin.y);
    			osgNativeLib.mouseButtonPressEvent(oneFingerOrigin.x,oneFingerOrigin.y, 3);
    			osgNativeLib.mouseMoveEvent(oneFingerOrigin.x,oneFingerOrigin.y);
    			
    		case MotionEvent.ACTION_MOVE:
    			float distance = sqrDistance(event);
    			float result = distance-distanceOrigin;
    			distanceOrigin=distance;
    			
    			if(result>1||result<-1){
    	    		oneFingerOrigin.y=oneFingerOrigin.y+result;
    				osgNativeLib.mouseMoveEvent(oneFingerOrigin.x,oneFingerOrigin.y);
    			}
    			
    			break;
    		case MotionEvent.ACTION_POINTER_UP:
    			mode =moveTypes.NONE;
    			osgNativeLib.mouseButtonReleaseEvent(oneFingerOrigin.x,oneFingerOrigin.y, 3);
    			break;
    		case MotionEvent.ACTION_UP:
    			mode =moveTypes.NONE;
    			osgNativeLib.mouseButtonReleaseEvent(oneFingerOrigin.x,oneFingerOrigin.y, 3);
    			break;
    		default :
    			Log.e(TAG,"2 point Action not captured");
    		}
    		break;    		
    	}
			
		return true;
	}

    //Ui Listeners
    OnClickListener uiListenerCenterView = new OnClickListener() {
        public void onClick(View v) {
        	//Log.d(TAG, "Center View");
        	osgNativeLib.keyboardDown(32);
        	osgNativeLib.keyboardUp(32);
        }
    };
    OnClickListener uiListenerChangeNavigation = new OnClickListener() {
        public void onClick(View v) {
        	//Log.d(TAG, "Change Navigation");
        	if(navMode==navType.PRINCIPAL){
        		msgUiNavSecondary.show();
        		navMode=navType.SECONDARY;
        	}
        	else{
        		msgUiNavPrincipal.show();
        		navMode=navType.PRINCIPAL;
        	}
        }
    };
    OnClickListener uiListenerChangeLight = new OnClickListener() {
        public void onClick(View v) {
        	//Log.d(TAG, "Change Light");
        	if(lightMode==lightType.ON){
        		msgUiLightOff.show();
        		lightMode=lightType.OFF;
        		osgNativeLib.keyboardDown(108);
            	osgNativeLib.keyboardUp(108);
        	}
        	else{
        		msgUiLightOn.show();
        		lightMode=lightType.ON;
        		osgNativeLib.keyboardDown(108);
            	osgNativeLib.keyboardUp(108);
        	}
        }
    };
    
    //Menu
    
    @Override
	public void colorChange(int color) {
		// TODO Auto-generated method stub
		// Do nothing
    	int red = Color.red(color);
    	int green = Color.green(color);
    	int blue = Color.blue(color);
    	//Log.d(TAG,"BACK color "+red+" "+green+" "+blue+" ");
    	osgNativeLib.setClearColor(red,green,blue);
	}
    
    //Android Life Cycle Menu
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.appmenu, menu);
        return super.onCreateOptionsMenu(menu);
    }
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle item selection
        switch (item.getItemId()) {
        case R.id.menuLoadObject:
        	Log.d(TAG,"Load Object");
        	loadLayerAddress.show();
            return true;
        case R.id.menuCleanScene:
        	Log.d(TAG,"Clean Scene");
        	osgNativeLib.clearContents();
            return true;
        case R.id.menuDeleteObject:
        	Log.d(TAG,"Delete a object");
        	String vNames[] = osgNativeLib.getObjectNames();
        	
        	//Remove Layer Dialog
    		AlertDialog.Builder removeLayerDialogBuilder = new AlertDialog.Builder(this);
    		removeLayerDialogBuilder.setTitle(R.string.uiDialogTextChoseRemove);
    		removeLayerDialogBuilder.setItems(vNames, new DialogInterface.OnClickListener() {
    			
    			@Override
    			public void onClick(DialogInterface dialog, int witch) {
    				// TODO Auto-generated method stub
    				osgNativeLib.unLoadObject(witch);
    			}
    		});
    		removeLayerDialog = removeLayerDialogBuilder.create();

    		if(vNames.length > 0)
    			removeLayerDialog.show();
        	
            return true;
        case R.id.menuChangeBackground:
        	Log.d(TAG,"Change background color");
        	int[] test = new int [3];
        	test = osgNativeLib.getClearColor();
        	backgroundColor = Color.rgb(test[0], test[1], test[2]);
        	
        	ColorPickerDialog colorDialog;
        	new ColorPickerDialog(this, this, backgroundColor).show();
            return true;
        case R.id.menuShowKeyboard:
        	Log.d(TAG,"Keyboard");
        	InputMethodManager mgr= (InputMethodManager)this.getSystemService(Context.INPUT_METHOD_SERVICE);
    		mgr.toggleSoftInput(InputMethodManager.SHOW_IMPLICIT, 0);
            return true;
        default:
            return super.onOptionsItemSelected(item);
        }
    }
    
    //Utilities
    /** Show an event in the LogCat view, for debugging */
    private void dumpEvent(MotionEvent event) {
       String names[] = { "DOWN", "UP", "MOVE", "CANCEL", "OUTSIDE",
             "POINTER_DOWN", "POINTER_UP", "7?", "8?", "9?" };
       StringBuilder sb = new StringBuilder();
       int action = event.getAction();
       int actionCode = action & MotionEvent.ACTION_MASK;
       sb.append("event ACTION_").append(names[actionCode]);
       if (actionCode == MotionEvent.ACTION_POINTER_DOWN
             || actionCode == MotionEvent.ACTION_POINTER_UP) {
          sb.append("(pid ").append(
                action >> MotionEvent.ACTION_POINTER_ID_SHIFT);
          sb.append(")");
       }
       sb.append("[");
       for (int i = 0; i < event.getPointerCount(); i++) {
          sb.append("#").append(i);
          sb.append("(pid ").append(event.getPointerId(i));
          sb.append(")=").append((int) event.getX(i));
          sb.append(",").append((int) event.getY(i));
          if (i + 1 < event.getPointerCount())
             sb.append(";");
       }
       sb.append("]");
       //Log.d(TAG, sb.toString());
    }
    private float sqrDistance(MotionEvent event) {
        float x = event.getX(0) - event.getX(1);
        float y = event.getY(0) - event.getY(1);
        return (float)(Math.sqrt(x * x + y * y));
     }
	
}