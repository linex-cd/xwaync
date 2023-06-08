
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 2048

int get_x11vnc_cmd(char* outbuf)
{
	
	FILE *fp = NULL;
    char buf[MAX_BUFFER_SIZE];

    // 清空buf
    memset(buf, 0, sizeof(buf));

    // 执行dmidecode命令并获取输出
    fp = popen("dmidecode -s system-family", "r");
    if (fp == NULL) {
        printf("Failed to run dmidecode command\n" );
        exit(1);
    }

    // 读取命令的输出
    while (fgets(buf, sizeof(buf)-1, fp) != NULL) {
        printf("%s", buf);
    }

    // 关闭文件指针
    pclose(fp);
	
	if(strstr(buf, "HUAWEI MateBook") == 0)
	{
		strcpy(outbuf, "x11vnc -display :0 -connect_or_exit repeater://%s+ID:%s -passwd %s -capslock -once -nomodtweak 2>&1");
		return 1;
	}
	
	
	//判断系统类型
	char osinfo[MAX_BUFFER_SIZE] = {0};
	
	FILE *f = fopen("/proc/version", "r");
	if(f)
	{

		fread(osinfo, 1, MAX_BUFFER_SIZE,f);

		fclose(f);

	}
	else{
		return 0;

	}



	if(strstr(osinfo, "KYLIN")!=0)
	{
		//麒麟无需获取缩放比例
		strcpy(outbuf, "x11vnc -display :0 -rawfb shm:$(ipcs -m | grep 7c7| awk '{print $2}')@$(xrandr -d :0 | grep '*' | cut -d' ' -f4)x32 -pipeinput UINPUT -cursor none -connect_or_exit repeater://%s+ID:%s -passwd %s -capslock -once -nomodtweak 2>&1");
		
		return 1;
	}
	
	if(strstr(osinfo, "phisik")!=0)
	{
		//获取UOS缩放比例
		FILE *fp = NULL;
		char buf[MAX_BUFFER_SIZE];

		// 清空buf
		memset(buf, 0, sizeof(buf));

		// 执行dmidecode命令并获取输出
		fp = popen("qdbus com.deepin.daemon.Appearance /com/deepin/daemon/Appearance com.deepin.daemon.Appearance.GetScaleFactor", "r");
		if (fp == NULL) {
			printf("Failed to run qdbus command\n" );
			exit(1);
		}

		// 读取命令的输出
		while (fgets(buf, sizeof(buf)-1, fp) != NULL) {
			printf("%s", buf);
		}

		// 关闭文件指针
		pclose(fp);
		//
		strcpy(outbuf, "x11vnc -display :0 -rawfb shm:$(ipcs -m | grep 7c7| awk '{print $2}')@$(xrandr -d :0 | grep '*' | cut -d' ' -f4)x32 -pipeinput UINPUT:accel=");
		
		//移除换行符后拼接
		buf[strlen(buf)-1] = '\0';
		strcat(outbuf, buf);
		
		strcat(outbuf, " -cursor none -connect_or_exit repeater://%s+ID:%s -passwd %s -capslock -once -nomodtweak 2>&1");
		
		return 1;
	}
	
	return 0;
	
}

//进程管理
#include <signal.h>
#include <pthread.h>
#include <libgen.h>

int kill_process(char *process_name, int exit_code)
{
	
    pid_t pid = -1;
    char command[50];
    sprintf(command, "pidof %s", process_name);

    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen failed");
		return 2;
    }

    // Get the process ID
    char pid_str[10];
    fgets(pid_str, 10, fp);
    pid = atoi(pid_str);

    if (pid == 0) {
        printf("Process %s is not running.\n", process_name);
    } else {
        printf("Process %s has ID %d.\n", process_name, pid);
		if (kill(pid, exit_code) == -1) {
			perror("kill failed");
			return 2;
		} else {
			printf("Process %s with ID %d killed.\n", process_name, pid);
			
			return 1;
		}
    }

    return 0;
}

#include <sys/ioctl.h>
#include <net/if.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <inttypes.h>



//获取网卡信息并生成唯一ID
char *get_mac_address() {
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    int success = 0;

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) {
        return NULL;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
        return NULL;
    }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it) {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            if (!(ifr.ifr_flags & IFF_LOOPBACK)) {
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    success = 1;
                    break;
                }
            }
        } else {
            return NULL;
        }
    }

    unsigned char mac_address[6];

    if (success) {
        memcpy(mac_address, ifr.ifr_hwaddr.sa_data, 6);
    }

    char *mac = malloc(18);
    sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
        mac_address[0], mac_address[1], mac_address[2],
        mac_address[3], mac_address[4], mac_address[5]);

    return mac;
}


uint64_t mac_to_uint64(const char *mac_address) {
    uint64_t val = 0;
    uint8_t bytes[6];

    if (6 == sscanf(mac_address, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
        &bytes[0], &bytes[1], &bytes[2], &bytes[3], &bytes[4], &bytes[5])) {
        val = ((uint64_t)bytes[0] << 40) | ((uint64_t)bytes[1] << 32) | ((uint64_t)bytes[2] << 24) |
              ((uint64_t)bytes[3] << 16) | ((uint64_t)bytes[4] << 8) | (uint64_t)bytes[5];
    }

    return val;
}

char* mac_to_9digit(const char *mac_address) {
    uint64_t mac_val = mac_to_uint64(mac_address);

    // Transform it to a 9-digit number
    mac_val = mac_val % 1000000000;

    char* result = malloc(10);
    sprintf(result, "%09" PRIu64, mac_val);

    return result;
}


//生成密码
#include <time.h>

char* generate_six_digit() {
    // 初始化随机数生成器的种子
    srand(time(NULL));

    // 生成一个在[100000, 999999]范围内的随机数
    int number = rand() % 900000 + 100000;

    char *result = malloc(7);
    sprintf(result, "%d", number);

    return result;
}



//服务


#include <gtk/gtk.h>
#include <glib/gprintf.h>


typedef struct {
    GtkWidget *window;
    GtkWidget *combo;
    GtkWidget *id_entry;
    GtkWidget *password_entry;
    GtkWidget *button;
	GtkWidget *timer_label;
} Widgets;



static volatile int flag_connected = 0;
static volatile int flag_ctrlC = 0;
static volatile int flag_btn = 0;


static int update_timer(gpointer data) {
	
	Widgets *w = (Widgets *)data;

	int elapsedTime = 0;
	

	while(flag_connected > 0) {
		

	
		elapsedTime++;
		
		int hours = elapsedTime / 3600;
		int minutes = (elapsedTime % 3600) / 60;
		int seconds = elapsedTime % 60;
		
		gchar* text = g_strdup_printf("已连接：%02d:%02d:%02d", hours, minutes, seconds);
		gtk_label_set_text(GTK_LABEL(w->timer_label), text);
		g_free(text);

		sleep(1);
	}
	
 
	flag_connected = 2;
	
	gtk_button_set_label(GTK_BUTTON(w->button), "启动服务");
	gtk_widget_set_sensitive(w->button, TRUE);
	
	gchar* text = g_strdup_printf("欢迎使用");
	gtk_label_set_text(GTK_LABEL(w->timer_label), text);
	g_free(text);

	
}



void msgbox(GtkWidget *w, char *msg) {
	 GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(w),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_CLOSE,
            msg);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
	
}

static int start_server(gpointer data) {

	printf("start_server...\n");

	Widgets *w = (Widgets *)data;
	
	gtk_widget_set_sensitive(w->button, FALSE);
	
    const gchar *id = gtk_entry_get_text(GTK_ENTRY(w->id_entry));
    const gchar *password = gtk_entry_get_text(GTK_ENTRY(w->password_entry));

    if (strlen(id) == 0 || strlen(password) == 0) {
        msgbox(w->window, "ID或者密码不能为空");
		gtk_button_set_label(GTK_BUTTON(w->button), "启动服务");
		gtk_widget_set_sensitive(w->button, TRUE);
        return 12;
    }
	
    

    GtkTreeIter iter;
	gchar *server;
    if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(w->combo), &iter)) {
        
        GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(w->combo));
        gtk_tree_model_get(model, &iter, 1, &server, -1);

    }
	

    char command[MAX_BUFFER_SIZE];
    printf("Connect: server=%s, id=%s, password=%s\n", server, id, password);
    printf("Running xwaync...\n");
	
	char cmd_fmt[MAX_BUFFER_SIZE] = {0};
	get_x11vnc_cmd(cmd_fmt);
	

    sprintf(command, cmd_fmt, server, id, password);
	puts(command);
	
	
    //system(command);
	//检查运行状态
    char buf[MAX_BUFFER_SIZE] = {0};
	FILE* fp = popen(command, "r");
	if (fp == NULL) {
		msgbox(w->window, "服务启动失败，请检查x11vnc是否安装");
		gtk_button_set_label(GTK_BUTTON(w->button), "启动服务");
		gtk_widget_set_sensitive(w->button, TRUE);
        return 3;
    }
	while (fgets(buf, MAX_BUFFER_SIZE, fp) != NULL) {
        printf("%s", buf);
		
		//判断被迫退出状态
		if(strstr(buf, "The VNC desktop is:"))
		{
			gchar* text = g_strdup_printf("准备就绪");
			gtk_label_set_text(GTK_LABEL(w->timer_label), text);
			g_free(text);
			
			gtk_button_set_label(GTK_BUTTON(w->button), "停止服务");
			gtk_widget_set_sensitive(w->button, TRUE);
			flag_btn = 1;
		}
		
		
	
		//判断状态
		if(strstr(buf, "Client Protocol Version"))
		{
			//已连接，启动计时器
			printf("NOW CONNECTED!\n");
			gtk_button_set_label(GTK_BUTTON(w->button), "断开连接");
			gtk_widget_set_sensitive(w->button, TRUE);
			flag_btn = 1;
			
			if(flag_connected == 0)//避免重复启动
			{
				
				flag_connected = 1;
	
				pthread_t timer_thread;
				pthread_create(&timer_thread, NULL, update_timer, data);

			}
			
		}
		
		//判断被迫退出状态
		if(strstr(buf, "caught signal: 2"))
		{
			//被强制终止
			printf("FIND： caught signal: 2\n");
			flag_ctrlC = 1;
			
			
		}
		
    }
	int ret = pclose(fp);
	

	//Ctrl+C断开，返回2
	if (flag_ctrlC > 0 && flag_connected == 0)
	{
		ret = 2;
	}
	
	//正常主动断开+密码错误断开
	if (flag_connected > 0 )
	{
		//
	}
	
	//点击远程结束断开
	if (flag_connected == 2 )
	{
		ret = 0;
	}
	
	int ret_new = ret;
	if (ret_new < 0 || ret_new > 255)
	{
		ret_new = ret_new % 255;
	}

	printf("%s => %d, %d\n", command, ret, ret_new);
   
	flag_btn = 0;
	flag_connected = 0;
	

	
	gtk_button_set_label(GTK_BUTTON(w->button), "启动服务");
	gtk_widget_set_sensitive(w->button, TRUE);
    
	gchar* text = g_strdup_printf("欢迎使用");
	gtk_label_set_text(GTK_LABEL(w->timer_label), text);
	g_free(text);
	

    printf("server finished.\n");
	
	

    return 0;
}




void on_button_clicked(GtkWidget *button, gpointer data) {
	
	kill_process("x11vnc", 2);
	
	if (flag_btn == 0)
	{
		pthread_t service_thread;
		pthread_create(&service_thread, NULL, start_server, data);
	}
	

}


int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    Widgets *w = g_new(Widgets, 1);
	
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "XWayNC GUI");
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	
	w->window = window;

    GtkWidget *grid = gtk_grid_new();
    //gtk_container_set_border_width(GTK_CONTAINER(grid), 80);
	gtk_widget_set_margin_top(GTK_WIDGET(grid), 60);
	gtk_widget_set_margin_bottom(GTK_WIDGET(grid), 40);
	gtk_widget_set_margin_left(GTK_WIDGET(grid), 80);
	gtk_widget_set_margin_right(GTK_WIDGET(grid), 80);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 30);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 30);
    gtk_container_add(GTK_CONTAINER(window), grid);

    GtkWidget *server_label = gtk_label_new("选择服务器:");
    gtk_grid_attach(GTK_GRID(grid), server_label, 0, 0, 1, 1);

    GtkListStore *store;
    GtkTreeIter iter;
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "NO.JS.CN", 1, "no.js.cn:9901", -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "测试服务器", 1, "172.20.96.192:9500", -1);
	gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "云上贵州互联网", 1, "111.123.243.77:5500", -1);
	gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "云上贵州政务网", 1, "172.18.21.176:5500", -1);
	

    w->combo = gtk_combo_box_new_with_model_and_entry(GTK_TREE_MODEL(store));
    gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(w->combo), 0);
    gtk_combo_box_set_active(GTK_COMBO_BOX(w->combo), 0);
    gtk_grid_attach(GTK_GRID(grid), w->combo, 1, 0, 1, 1);

    GtkWidget *id_label = gtk_label_new("我的ID:");
    gtk_grid_attach(GTK_GRID(grid), id_label, 0, 1, 1, 1);

    w->id_entry = gtk_entry_new();
    //获取机器的UUID
	char* mac_address = get_mac_address();
	char* id = mac_to_9digit(mac_address);
	gtk_entry_set_text(GTK_ENTRY(w->id_entry), id);
	gtk_editable_set_editable(GTK_EDITABLE(w->id_entry), FALSE);
	free(mac_address);
	free(id);
	gtk_grid_attach(GTK_GRID(grid), w->id_entry, 1, 1, 1, 1);


    GtkWidget *password_label = gtk_label_new("访问密码:");
    gtk_grid_attach(GTK_GRID(grid), password_label, 0, 2, 1, 1);

    w->password_entry = gtk_entry_new();
	char* password = generate_six_digit();
	gtk_entry_set_text(GTK_ENTRY(w->password_entry), password);
	free(password);
	gtk_grid_attach(GTK_GRID(grid), w->password_entry, 1, 2, 1, 1);

    w->button = gtk_button_new_with_label("启动服务");
    g_signal_connect(w->button, "clicked", G_CALLBACK(on_button_clicked), w);
    gtk_grid_attach(GTK_GRID(grid), w->button, 0, 3, 2, 1);
	
	w->timer_label = gtk_label_new("欢迎使用");
	gtk_grid_attach(GTK_GRID(grid), w->timer_label, 0, 4, 2, 1);


	gtk_widget_grab_focus(w->button);
	
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);
    gtk_main();

    g_free(w);

    return 0;
}
