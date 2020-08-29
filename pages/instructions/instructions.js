


Page({

  /**
   * 页面的初始数据
   */
  data: {
    deviceId: '',
    serviceId: '',
    characteristicId: '',
    transDeviceId: '',
    feedbackValue: []
    

  },
  writeBLECharacteristicValue1() {
    // 向蓝牙设备发送一个0x00的16进制数据
    var senddata = 'QGled1 open/'
    let buffer = new ArrayBuffer(senddata.length)
    let dataView = new DataView(buffer)
    for (var i = 0; i < senddata.length; i++) {
      dataView.setUint8(i, senddata.charAt(i).charCodeAt())
    }


    wx.writeBLECharacteristicValue({
      deviceId: this.data.deviceId,                     //'F8:33:31:37:9A:AB',
      serviceId: this.data.serviceId,                 //'0000FFE0-0000-1000-8000-00805F9B34FB',
      characteristicId: this.data.characteristicId,          //'0000FFE1-0000-1000-8000-00805F9B34FB',
      value: buffer,
      success(res) {
        console.log('writeBLECharacteristicValue success', res)

      },
      fail: function (res) {
        console.log('失败', res)
      }
    })
  },
  writeBLECharacteristicValue2() {
    // 向蓝牙设备发送一个0x00的16进制数据
    var senddata = 'QGled1 close/'
    let buffer = new ArrayBuffer(senddata.length)
    let dataView = new DataView(buffer)
    for (var i = 0; i < senddata.length; i++) {
      dataView.setUint8(i, senddata.charAt(i).charCodeAt())
    }


    wx.writeBLECharacteristicValue({
      deviceId: this.data.deviceId,                     //'F8:33:31:37:9A:AB',
      serviceId: this.data.serviceId,                 //'0000FFE0-0000-1000-8000-00805F9B34FB',
      characteristicId: this.data.characteristicId,          //'0000FFE1-0000-1000-8000-00805F9B34FB',
      value: buffer,
      success(res) {
        console.log('writeBLECharacteristicValue success', res)
      },
      fail: function (res) {
        console.log('失败', res)
      }
    })
  },
  writeBLECharacteristicValue3() {
    // 向蓝牙设备发送一个0x00的16进制数据
    var senddata = 'QGled2 open/'
    let buffer = new ArrayBuffer(senddata.length)
    let dataView = new DataView(buffer)
    for (var i = 0; i < senddata.length; i++) {
      dataView.setUint8(i, senddata.charAt(i).charCodeAt())
    }


    wx.writeBLECharacteristicValue({
      deviceId: this.data.deviceId,                     //'F8:33:31:37:9A:AB',
      serviceId: this.data.serviceId,                 //'0000FFE0-0000-1000-8000-00805F9B34FB',
      characteristicId: this.data.characteristicId,          //'0000FFE1-0000-1000-8000-00805F9B34FB',
      value: buffer,
      success(res) {
        console.log('writeBLECharacteristicValue success', res)
      },
      fail: function (res) {
        console.log('失败', res)
      }
    })
  },
  writeBLECharacteristicValue4() {
    // 向蓝牙设备发送一个0x00的16进制数据
    var senddata = 'QGled2 close/'
    let buffer = new ArrayBuffer(senddata.length)
    let dataView = new DataView(buffer)
    for (var i = 0; i < senddata.length; i++) {
      dataView.setUint8(i, senddata.charAt(i).charCodeAt())
    }


    wx.writeBLECharacteristicValue({
      deviceId: this.data.deviceId,                     //'F8:33:31:37:9A:AB',
      serviceId: this.data.serviceId,                 //'0000FFE0-0000-1000-8000-00805F9B34FB',
      characteristicId: this.data.characteristicId,          //'0000FFE1-0000-1000-8000-00805F9B34FB',
      value: buffer,
      success(res) {
        console.log('writeBLECharacteristicValue success', res)
      },
      fail: function (res) {
        console.log('失败', res)
      }
    })
  },
  feedback: function () {
    var that = this

    // 启用 notify 功能
    wx.notifyBLECharacteristicValueChanged({
      state: true,
      deviceId: this.data.deviceId,        //'F8:33:31:37:9A:AB',
      serviceId: this.data.serviceId,      //'0000FFE0-0000-1000-8000-00805F9B34FB',
      characteristicId: this.data.characteristicId,
      success: function () {
        console.log("监听特征值成功")
      }
    })
    // 启用 notify 功能
    // ArrayBuffer转为16进制数
    function ab2hex(buffer) {
      var hexArr = Array.prototype.map.call(
        new Uint8Array(buffer),
        function (bit) {
          return ('00' + bit.toString(16)).slice(-2)
        }
      )
      return hexArr.join('');
    }
    // 16进制数转ASCLL码
    function hexCharCodeToStr(hexCharCodeStr) {
      var trimedStr = hexCharCodeStr.trim();
      var rawStr = trimedStr.substr(0, 2).toLowerCase() === "0x" ? trimedStr.substr(2) : trimedStr;
      var len = rawStr.length;
      var curCharCode;
      var resultStr = [];
      for (var i = 0; i < len; i = i + 2) {
        curCharCode = parseInt(rawStr.substr(i, 2), 16);
        resultStr.push(String.fromCharCode(curCharCode));
      }
      return resultStr.join("");
    }
    //监听回调，接收数据
    wx.onBLECharacteristicValueChange(function (characteristic) {
      var hex = ab2hex(characteristic.value)
      that.setData({
        feedbackValue: (parseInt(hexCharCodeToStr(hex),16)) / 256 * 10, //提取数字  .match(/\d+(.\d+)?/g)
        // feedbackValueD: parseInt(that.feedbackValue,16)
        

      })
      
    })
  },

  fail: function (res) {
    that.setData({
      connected: false
    })
  },
  
  
  /**
   * 生命周期函数--监听页面加载
   */
  onLoad: function (option) {
    // console.log(option.query)
    let that = this
    const eventChannel = this.getOpenerEventChannel()
    eventChannel.emit('acceptDataFromOpenedPage', { data: 'test' });
    eventChannel.emit('someEvent', { data: 'test' });
    // 监听acceptDataFromOpenerPage事件，获取上一页面通过eventChannel传送到当前页面的数据
    eventChannel.on('acceptDataFromOpenerPage', data => {
      that.setData({
        deviceId: data.deviceId,
        serviceId: data.serviceId,
        characteristicId: data.characteristicId
      });
      
      //   console.log(that.data);
      //   console.log(that.data.deviceId)

      // console.log(eventChannel);

    })

  },
  

  /**
   * 生命周期函数--监听页面初次渲染完成
   */
  onReady: function () {

  },

  /**
   * 生命周期函数--监听页面显示
   */
  onShow: function () {

  },

  /**
   * 生命周期函数--监听页面隐藏
   */
  onHide: function () {

  },

  /**
   * 生命周期函数--监听页面卸载
   */
  onUnload: function () {

  },

  /**
   * 页面相关事件处理函数--监听用户下拉动作
   */
  onPullDownRefresh: function () {

  },

  /**
   * 页面上拉触底事件的处理函数
   */
  onReachBottom: function () {

  },

  /**
   * 用户点击右上角分享
   */
  onShareAppMessage: function () {

  }
})