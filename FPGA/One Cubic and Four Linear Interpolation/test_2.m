% 读取图像
%img = imread('h_480_368.jpg');
% 读取输入图像
input_img = imread('h_480_368.jpg');

% 缩放因子
scale_factor = 2; % 例如放大2倍

% 使用改进的双三次插值函数进行缩放
output_img = modified_bicubic_interpolation(input_img, scale_factor);

% 将原始图像和缩放后的图像显示在同一窗口中，以便进行对比
figure;

% 显示原始图像
subplot(1, 3, 1);
imshow(input_img);
title('原始图像');

% 显示放大后的图像
subplot(1, 3, 2);
imshow(output_img);
title('放大后的图像');

% 使用 `imshowpair` 函数显示原始图像和放大后图像的差异
subplot(1, 3, 3);
imshowpair(imresize(input_img, scale_factor), output_img, 'diff');
title('放大前后差异');

% 调整图像对比度，突出差异部分（可选）
colormap(gca, 'hot'); % 使用伪彩色显示差异部分，便于观察

% 保存差异图像（可选）
imwrite(double(imresize(input_img, scale_factor)) - double(output_img), 'difference_image.png');

