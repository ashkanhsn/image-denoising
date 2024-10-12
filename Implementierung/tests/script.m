
% MATLAB script to generate test data to check the correctness of the functions

% laplace convolution
function result = laplace(original)
    result = conv2(original, [0, 1, 0; 1, -4, 1; 0, 1, 0], 'same');
    result = uint8(abs(result) / 4);
end

% blur convolution
function result = blur(original)
    result = conv2(original, [1, 2, 1; 2, 4, 2; 1, 2, 1], 'same');
    result = uint8(result / 16);
end

% combine
function result = combine(original, laplace, blur)
    sizeMatrix = size(original);
    width = sizeMatrix(2);
    height = sizeMatrix(1);

    result = zeros(sizeMatrix, 'uint8');

    for i = 1:height
        for j = 1:width
            result(i, j) = uint8((laplace(i, j) * original(i, j) + (255 - laplace(i, j)) * blur(i, j)) / 255);
        end
    end
end