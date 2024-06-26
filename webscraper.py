"""
Author: Hiroshi Thomas
Date: 4/20/24
Description: Web Scraper

This script is licensed under the MIT License.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import tkinter as tk
from tkinter import filedialog, messagebox
import requests
from bs4 import BeautifulSoup
import pandas as pd

class WebScraperApp:
    def __init__(self, root):
        # Initialize the main application window
        self.root = root
        self.root.title("Web Scraper")

        # Create and place URL label and entry field
        self.url_label = tk.Label(root, text="URL:")
        self.url_label.grid(row=0, column=0, padx=5, pady=5)
        self.url_entry = tk.Entry(root, width=50)
        self.url_entry.grid(row=0, column=1, padx=5, pady=5)

        # Create and place tag label and entry field
        self.tag_label = tk.Label(root, text="Tag to scrape:")
        self.tag_label.grid(row=1, column=0, padx=5, pady=5)
        self.tag_entry = tk.Entry(root, width=50)
        self.tag_entry.grid(row=1, column=1, padx=5, pady=5)

        # Create and place the scrape button
        self.scrape_button = tk.Button(root, text="Scrape", command=self.scrape)
        self.scrape_button.grid(row=2, column=0, columnspan=2, padx=5, pady=5)

    def scrape(self):
        # Get the URL and tag from the entry fields
        url = self.url_entry.get()
        tag = self.tag_entry.get()

        # Check if both URL and tag are provided
        if not url or not tag:
            messagebox.showwarning("Input Error", "Please enter both URL and tag.")
            return

        # Try to fetch the webpage content
        try:
            response = requests.get(url)
            response.raise_for_status()  # Raise an error for bad status codes
        except requests.RequestException as e:
            messagebox.showerror("Request Error", f"Failed to retrieve URL: {e}")
            return

        # Parse the webpage content
        soup = BeautifulSoup(response.text, 'html.parser')
        elements = soup.find_all(tag)

        # Check if any elements were found
        if not elements:
            messagebox.showinfo("No Data", "No elements found with the specified tag.")
            return

        # Extract text content from the elements
        data = [element.get_text(strip=True) for element in elements]
        self.save_to_csv(data)

    def save_to_csv(self, data):
        # Open a file dialog to choose where to save the CSV file
        filepath = filedialog.asksaveasfilename(defaultextension=".csv", filetypes=[("CSV files", "*.csv")])
        if filepath:
            # Create a DataFrame and save it as a CSV file
            df = pd.DataFrame(data, columns=["Content"])
            df.to_csv(filepath, index=False)
            messagebox.showinfo("Success", "Data saved successfully.")

if __name__ == "__main__":
    # Create the main window and run the application
    root = tk.Tk()
    app = WebScraperApp(root)
    root.mainloop()
