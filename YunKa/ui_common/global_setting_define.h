#ifndef _GLOBAL_SETTING_DEFINE_
#define _GLOBAL_SETTING_DEFINE_
#pragma once

#define  _DEBUG_DATA_

#define   WND_X                          1024            
#define   WND_Y                          700


    
#define   WM_MENU_START                  WM_USER+3000

#define   WM_SCREEN_CAPTURE_SUCCED       WM_USER+3200



/************************************************ ��ʱ������  *****************************************************************************************************************************************************/
//��ʱ�������߷ÿ͵Ķ�ʱ��
#define     WM_ADD_ONLINE_DATA_TIMER_ID                                  1
#define     DELAY_ADD_ONLINE_DATA_TIME                                   2000



//�����ַ�����
#define   ONLINE_TEXT                         _T("(����)")
#define   OFFLINE_TEXT                        _T("(����)")   
 
#define   DEFAULT_TREE_TYPE_1                  _T("{x 4}{i gameicons.png 18 0}{x 4}")



//ȫ���ַ������� 
//Сͼ��
#define    DEFINE_SMALL_ICON_PATH                                               _T("E:\\Moliyun\\bin\\SkinRes\\small.ico")





//��Ϣ����
#define WM_INIT_ROOM_USER_LIST_MSG                                                WM_USER+100
#define WM_GET_MYSELF_INFO_MSG                                                    WM_USER+101
#define WM_UPDATE_SELECT_USER_MSG                                                 WM_USER+102






/*login define****************************************************************************************************************************************************************/
#define  DEBUG_USERNAME_TEXT                                                         _T("9692110")         
#define  DEBUG_PASSWORD_TEXT                                                         _T("123")
#define  DEBUG_HOST_ADDRESS                                                          ""
#define  _GLOBAL_LOGIN_UI_CONFIG_FILE                                             _T("login\\login.xml")  // _T("login\\login.xml")


//menu
#define WM_MY_MESSAGE_NOTIFYICON													WM_USER+1
#define  IDM_MENU_QUIT																WM_USER+10
#define  IDM_MENU_RELOGIN															WM_USER+11

//user click menu
#define  MENU_CLICK_USER_TO_SPEAK                                                  WM_USER+20
#define  MENU_FIND_ONE_USER_INFO                                                   WM_USER+21

//login wnd control
#define     DEF_CANCEL_WND_BUTTON													_T("cancelBtn")
#define     DEF_LOGIN_WND_BUTTON													_T("loginBtn")
#define     DEF_ACCOUNT_ID_COMBO													_T("accountCombo")
#define     DEF_ACCOUNT_ID_EDIT														_T("accountEdit")
#define     DEF_PASSWORD_TEXT_EDIT													_T("pwdEdit")

#define     DEF_CLOSE_WND_BUTTON													_T("closeBtn")
#define     DEF_CLOSE_WND_BUTTON_2													_T("closebtn2")
#define     DEF_MIN_WND_BUTTON														_T("minBtn")
#define     DEF_RESTORE_WND_BUTTON													_T("restoreBtn")
#define     DEF_MAX_WND_BUTTON														_T("maxBtn")










/*end login define***************************************************************************************************************************************************************/







#endif