/*
 * Author: Hiroshi Thomas
 * Date: 4/24/24
 * Description: Web Scraper
 * 
 * This script is licensed under the MIT License.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <gtk/gtk.h>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *memory;
    size_t size;
} MemoryStruct;

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        return 0;  // out of memory
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

static void scrape_url(const char *url, const char *tag, const char *filename) {
    CURL *curl_handle;
    CURLcode res;

    MemoryStruct chunk;
    chunk.memory = malloc(1);  // initial size
    chunk.size = 0;  // no data yet

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    res = curl_easy_perform(curl_handle);

    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        // Parse the HTML content
        htmlDocPtr doc = htmlReadMemory(chunk.memory, chunk.size, NULL, NULL, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
        if (doc == NULL) {
            fprintf(stderr, "Failed to parse HTML\n");
        } else {
            // XPath evaluation to find the tag
            xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
            char xpathExpr[256];
            snprintf(xpathExpr, sizeof(xpathExpr), "//%s", tag);
            xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((xmlChar *)xpathExpr, xpathCtx);

            if (xpathObj == NULL) {
                fprintf(stderr, "Failed to evaluate XPath\n");
            } else {
                FILE *file = fopen(filename, "w");
                if (file == NULL) {
                    perror("fopen");
                } else {
                    xmlNodeSetPtr nodes = xpathObj->nodesetval;
                    for (int i = 0; i < nodes->nodeNr; i++) {
                        xmlChar *content = xmlNodeGetContent(nodes->nodeTab[i]);
                        fprintf(file, "%s\n", content);
                        xmlFree(content);
                    }
                    fclose(file);
                }
                xmlXPathFreeObject(xpathObj);
            }
            xmlXPathFreeContext(xpathCtx);
            xmlFreeDoc(doc);
        }
    }

    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();
}

static void on_scrape_button_clicked(GtkButton *button, gpointer user_data) {
    GtkEntry **entries = (GtkEntry **)user_data;
    const char *url = gtk_entry_get_text(entries[0]);
    const char *tag = gtk_entry_get_text(entries[1]);
    const char *filename = gtk_entry_get_text(entries[2]);

    scrape_url(url, tag, filename);

    GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Scraping complete!");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Web Scraper");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    GtkWidget *url_label = gtk_label_new("URL:");
    GtkWidget *url_entry = gtk_entry_new();

    GtkWidget *tag_label = gtk_label_new("Tag to scrape:");
    GtkWidget *tag_entry = gtk_entry_new();

    GtkWidget *filename_label = gtk_label_new("Save to file:");
    GtkWidget *filename_entry = gtk_entry_new();

    GtkWidget *scrape_button = gtk_button_new_with_label("Scrape");

    GtkEntry *entries[] = {GTK_ENTRY(url_entry), GTK_ENTRY(tag_entry), GTK_ENTRY(filename_entry)};
    g_signal_connect(scrape_button, "clicked", G_CALLBACK(on_scrape_button_clicked), entries);

    gtk_grid_attach(GTK_GRID(grid), url_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), url_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), tag_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), tag_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), filename_label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), filename_entry, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), scrape_button, 0, 3, 2, 1);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
