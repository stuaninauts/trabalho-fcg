# Trabalho Final de Fundamentos de Computação Gráfica

## Contribuição de cada membro e descrição do processo desenvolvimento


- - [X] Objetos virtuais representados através de malhas poligonais complexas (malhas de triângulos).
	- Os modelos com complexidade igual ou maior que "cow.obj": objeto bônus (`"bonus-compressed.obj"`), carro (`"supratunadov5.obj"`) e árvore (`"tree.obj"`)
	- O modelo original do carro foi obtido neste [link](https://www.cgtrader.com/free-3d-models/car/racing-car/toyota-supra-mk4-c154dbfb-31b0-403f-affb-b30e1c5b34f1) porém foram feitas modificações utilizando o software Blender: rebaixamento do carro, cambagem das rodas, separação de componentes, aumento do escapamento e substituição das logos do carro por essa desse [link](https://www.cgtrader.com/items/2072818/download-page)
	- Integrante responsável: Eduardo
		
- [X] Transformações geométricas de objetos virtuais.
	- As teclas "W" e "S" possibilitam a movimentação dianteira e traseira do carro com translações no carro de acordo com a sua direção. Essas teclas também culminam na rotação das 4 rodas de acordo com a velocidade do carro
	- As teclas "A" e "D" realizam transformações nas rodas dianteiras que posteriormente vão ditar a direção do carro de acordo com a sua rotação. 
	- Integrante responsável: Eduardo

- [X] Controle de câmeras virtuais.
    - A tecla "L" permite a alternância para a câmera look-at (visão em terceira pessoa, focada no carro, alternando sua rotação baseada na rotação do carro com a possibilidade de alterar o "zoom" com a tecla "SCROLL") e a câmera livre (teclas "UP", "DOWN", "LEFT", "RIGHT" permitem a movimentação desta câmera)
    - Integrante responsável: Gustavo

- [X] No mínimo um objeto virtual deve ser copiado com duas ou mais instâncias, isto é, utilizando duas ou mais 
    - Os seguintes objetos possuem duas ou mais instâncias: outdoor (`"outdoor.obj"`), bônus (`"bonus-compressed.obj"`) e árvore (`"tree.obj"`)
    - Integrantes responsáveis:  Eduardo e Gustavo

- [X] Testes de intersecção entre objetos virtuais.
    - TODO
    No mínimo sua aplicação deve utilizar três tipos de teste de intersecção (por exemplo, um teste cubo-cubo, um teste cubo-plano, e um teste ponto-esfera).
    Estes testes devem ter algum propósito dentro da lógica de sua aplicação.
    Por exemplo, em um jogo de corrida, o modelo virtual de um carro não pode atravessar a parede, e para tanto é necessário testar a intersecção entre estes dois objetos de modo a evitar esta intersecção.
    Os testes de colisão devem ser implementados em um arquivo à parte, nomeado "collisions.cpp".
    - Integrante responsável: Gustavo

- [X] Modelos de iluminação de objetos geométricos.
    - Modelos de iluminação: a árvore (`"tree.obj"`) e o outdoor (`"outdoor.obj"`) possuem iluminação difusa e os demais objetos seguem o modelo de Blinn-Phong
    - Modelos de interpolação para iluminação: o objeto bônus (`"bonus-compressed.obj"`) utiliza o modelo de Gouraud e os demais objetos seguem o modelo de Phong
    - Integrantes responsáveis: Eduardo e Gustavo

- [X] Mapeamento de texturas
    - Todos objetos foram mapeados com texturas personalizadas (baixadas da internet) independentes dos arquivos obj 
    - Integrante responsável: Eduardo

- [X] Curvas de Bézier.
    - TODO
    No mínimo um objeto virtual de sua aplicação deve ter sua movimentação definida através de uma curva de Bézier cúbica. O objeto deve se movimentar de forma suave ao longo do espaço em um caminho curvo (não reto).
    - Integrante responsável: Gustavo

- [X] Animação de Movimento baseada no tempo.
    - A movimentação do carro (incluindo a rotação das rodas) e a do objeto bônus são baseadas no tempo
    - Integrantes responsáveis: Eduardo e Gustavo

## Utilização do ChatGPT
Durante o desenvolvimento do trabalho, utilizamos o ChatGPT como ferramenta de auxílio em algumas tarefas, como brainstorming de ideias, suporte na sintaxe da linguagem C++ e resolução de pequenos bugs.

No que diz respeito à implementação do código, tentamos utilizá-lo para nos ajudar a implementar a movimentação do carro. Contudo, ele falhou em todas as tentativas, retornando trechos de código não funcionais e sem sentido. Após conseguirmos implementar a movimentação básica do carro, o ChatGPT foi útil para nos ajudar a compreender a lógica necessária para adicionar uma mecânica de "drift", deixando o carro deslizar mais. Também buscamos sua ajuda para criar o "skybox" do jogo, mas, mais uma vez, as sugestões foram inadequadas, o que nos levou a implementar essa funcionalidade integralmente por conta própria.

Por outro lado, ele foi bastante eficaz ao nos orientar no uso do Blender, um software que nenhum de nós havia utilizado antes. Ele nos ajudou a manusear o programa e a desenvolver scripts para criar objetos dentro do Blender. Por exemplo, o objeto `"outdoor.obj"` foi criado inteiramente com um script gerado com sua assistência, enquanto o `"track.obj"` foi desenvolvido por meio de um script que convertia um arquivo SVG em um objeto no Blender.

## Imagens do jogo 


## Manual de uso
O objetivo do jogo é gerar a maior pontuação possível. Para isso é necessário se manter na pista e não colidir com objetos fora dela. Quanto maior a velocidade mais pontos são gerados.
Os objetos localizados nas curvas são objetos bônus multiplicadores de pontuação, ou seja, quando o carro passa por cima deles o multiplicador de pontuação aumenta, resultado em mais pontos.

**Controles de Movimentação**
- **W**: Move o carro para frente.
- **S**: Move o carro para trás.
- **A**: Move o carro para a esquerda.
- **D**: Move o carro para a direita.

**Controles de Câmera**
- **L**: Alterna entre os modos de câmera:
  - **Câmera em terceira pessoa**: Focada no carro.
  - **Câmera livre**: Permite movimentação independente da câmera utilizando as **setas do teclado**.

**Outras Funções**
- **Espaço (Space)**: Reinicia o jogo desde o início.

## Compilação e execução

Para rodar o código basta clonar este repositório e executar os seguintes comandos
sh```
cd trabalho-fcg
make
make run
```

	
