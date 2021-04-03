/*
 * @Author: your name
 * @Date: 2021-03-21 10:43:34
 * @LastEditTime: 2021-03-21 16:56:55
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /quickjs/tmp.js
 */

function solution(n) {
  const dp = new Array(n + 1).fill(false);
  dp[0] = true;
  for (let i = 1; i <= n; i++) {
    if (i - 7 >= 0) {
      dp[i] = dp[i - 7] || dp[i];
    }
    if ( i - 13 >= 0) {
      dp[i] = dp[i - 13] || dp[i];
    }
    if ( i - 29 >= 0) {
      dp[i] = dp[i - 29] || dp[i];
    }
  }
  return dp[n];
}

//方法二：动态规划 自顶向下
const dp = new Array(n + 1).fill(-1);
dp[0] = 1;
function dps(n) {
  if (n == 0 || dp[n] == 1) {
    return true;
  }
  if (n < 0 || dp[n] == 0) {
    return false;
  }
  if (dps[n - 7] || dps[n - 13] || dps[n - 29]) {
    dp[n] = 1;
  } else {
    dp[n] = 0;
  }
  return dp[n] == 1;
}