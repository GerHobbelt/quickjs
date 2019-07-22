/*
 * Copyright 2019 Hippo Seven
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.hippo.quickjs.android.test

import android.app.Activity
import android.os.Bundle
import android.view.Menu
import android.widget.Toast

class TestActivity : Activity() {

  private lateinit var logView: LogView

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)

    logView = LogView(this)
    setContentView(logView)
    (application as App).tester.registerMessageQueuePrinter(logView)
  }

  override fun onCreateOptionsMenu(menu: Menu?): Boolean {
    menu?.add("Send Log File")?.apply {
      setOnMenuItemClickListener {
        val tester = (application as App).tester
        if (tester.isFinished) {
          tester.shareLogFile()
        } else {
          Toast.makeText(this@TestActivity, "The test is not finished", Toast.LENGTH_SHORT).show()
        }
        true
      }
    }
    return true
  }

  override fun onDestroy() {
    super.onDestroy()

    logView.close()
  }
}
