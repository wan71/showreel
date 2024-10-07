function output_img = modified_bicubic_interpolation(input_img, scale_factor)
    % 获取输入图像大小
    [in_height, in_width, num_channels] = size(input_img);
    
    % 计算输出图像的大小
    out_height = round(in_height * scale_factor);
    out_width = round(in_width * scale_factor);
    
    % 初始化输出图像
    output_img = zeros(out_height, out_width, num_channels, 'uint8');
    
    % 计算缩放比例的倒数
    scale_inv = 1 / scale_factor;
    
    % 遍历输出图像的每个像素
    for channel = 1:num_channels
        for y_out = 1:out_height
            for x_out = 1:out_width
                % 计算在原图像中的位置
                x_in = (x_out - 0.5) * scale_inv + 0.5;
                y_in = (y_out - 0.5) * scale_inv + 0.5;
                
                % 找到周围的 8 个像素点
                x_base = floor(x_in) - 1;
                y_base = floor(y_in);
                
                % 计算垂直方向的线性插值，得到 S0, S1, S2, S3
                S = zeros(1, 4);
                for m = 0:3
                    x_src = x_base + m;
                    
                    % 边界检查
                    if x_src < 1
                        x_src = 1;
                    elseif x_src > in_width
                        x_src = in_width;
                    end
                    
                    % 获取 y0 和 y1 处的像素值
                    y0 = max(1, min(y_base, in_height - 1));
                    y1 = min(y0 + 1, in_height);
                    
                    p0 = double(input_img(y0, x_src, channel));
                    p1 = double(input_img(y1, x_src, channel));
                    
                    % 垂直方向线性插值
                    t = y_in - y0;
                    S(m + 1) = (1 - t) * p0 + t * p1;
                end
                
                % 使用 S0, S1, S2, S3 进行水平方向的三次插值
                t = x_in - (x_base + 1);
                interpolated_value = cubic_weight(S, t);
                
                % 赋值给输出图像
                output_img(y_out, x_out, channel) = uint8(min(max(interpolated_value, 0), 255));
            end
        end
    end
end

function value = cubic_weight(S, t)
    % 三次插值权重计算函数
    % S 为 4 个水平插值得到的值 (S0, S1, S2, S3)
    % t 为插值位置的偏移量
    
    % 计算权重
    value = S(1) * cubic_kernel(t + 1) + ...
            S(2) * cubic_kernel(t) + ...
            S(3) * cubic_kernel(t - 1) + ...
            S(4) * cubic_kernel(t - 2);
end

function w = cubic_kernel(x)
    % 三次插值核函数
    % 根据 x 的位置计算权重
    x = abs(x);
    if x < 1
        w = 1.5 * x^3 - 2.5 * x^2 + 1;
    elseif x < 2
        w = -0.5 * x^3 + 2.5 * x^2 - 4 * x + 2;
    else
        w = 0;
    end
end
