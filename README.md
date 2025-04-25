ĐỀ TÀI: THÙNG RÁC THÔNG MINH

Platform triển khai: openHAB thông qua giao thức MQTT

Nhóm 4 BDU thành viên gồm : Lê Duy Tuấn Anh 23050098 và Lý Lâm Vũ 23050102

- Thiết bị sẽ sử dụng: 
•	ESP8266 Node MCU V3
•	Cảm biến siêu âm HC-SR04
•	Cảm biến độ ẩm đất
•	Động cơ Servo SG90
•	Đèn LED 2 chân
•	Còi Buzzer
•	Dây cắm Breadboard (đực - đực, cái - cái, đực - cái)
•	Dây USB

-	Nguyên lý hoạt động:
Khi có rác được đưa vào thùng:
•	Hệ thống sử dụng cảm biến siêu âm để phát hiện khoảng cách đến vật thể.
•	Nếu có rác (khoảng cách dưới 15 cm), hệ thống sẽ tiến hành đo độ ẩm của rác.
•	Sau đó sẽ báo còi và nháy đèn LED
•	Dựa vào độ ẩm thu được:
o	Nếu rác ướt, servo nghiêng phải -> Còi báo và đèn chớp 3 lần
o	Nếu rác khô, servo nghiêng trái -> Còi báo và đèn chớp 3 lần
•	Dữ liệu về khoảng cách, độ ẩm, loại rác và trạng thái servo, số lần mở nắp (nghiêng trái/phải) sẽ được gửi về máy chủ OpenHAB thông qua giao thức MQTT.
•	Người dùng có thể theo dõi thông tin này qua giao diện dashboard, đồng thời có thể gửi lệnh điều khiển nắp (servo) và đèn, còi từ xa nếu muốn.
