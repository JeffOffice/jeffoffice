# Transições entre diapositivos
<!-- keywords: transição, transições, transformação, morph, desvanecer, empurrar, revelar gradualmente, dividir, revelar, cobrir, descobrir, zoom, cortar, opções de efeito, objetos, palavras, carateres, corresponder, par, nome, direção, duração, avançar diapositivo, ao clicar com o rato, após, automático, temporização, aplicar a todos, pré-visualizar, estrela -->

Uma **transição** é o efeito reproduzido quando a apresentação de diapositivos passa de um diapositivo para o seguinte. Uma transição pertence ao diapositivo em que entra. A apresentação de diapositivos reprodu-la quando avança para esse diapositivo.

## Adicionar uma transição

1. Clique no diapositivo no painel de diapositivos.
2. Clique no separador **Transições**. Na galeria **Transição para Este Diapositivo**, clique num efeito: **Nenhuma**, **Transformação**, **Desvanecer**, **Empurrar**, **Revelar Gradualmente**, **Dividir**, **Revelar**, **Cobrir**, **Descobrir**, **Zoom** ou **Cortar**. A imagem do diapositivo no painel de diapositivos recebe uma pequena estrela para indicar que tem uma transição.
3. Clique em **Transições > Pré-visualizar > Pré-visualizar** para a ver reproduzida na entrada do diapositivo, sobre a área do diapositivo. Prima Esc para a parar.

Escolha **Nenhuma** para remover uma transição.

## Escolher como decorre

Clique em **Opções de Efeito** para ver as opções do efeito:

- **Empurrar**, **Revelar Gradualmente**, **Revelar**, **Cobrir** e **Descobrir** têm uma direção: **Da direita**, **Da esquerda**, **De baixo** ou **De cima**.
- **Dividir** e **Zoom** funcionam **Para fora** ou **Para dentro**.
- **Transformação** move **Objetos**, **Palavras** ou **Carateres**. Veja abaixo.
- **Desvanecer** e **Cortar** não têm opções. Um corte passa pelo preto.

## Transformação

A **Transformação** move o que dois diapositivos têm em comum. Um **objeto** é um elemento de um diapositivo: uma forma, uma imagem, uma caixa de texto, uma linha ou uma tabela. A apresentação passa de um diapositivo para o seguinte. Cada objeto que está nos dois diapositivos desliza do local onde está no primeiro diapositivo para o local onde está no segundo. Pelo caminho, aumenta ou diminui, roda e muda de cor. Um objeto que está apenas no primeiro diapositivo desvanece-se. Um objeto que está apenas no segundo diapositivo aparece gradualmente.

Para criar uma transformação:

1. Clique com o botão direito do rato num diapositivo no painel de diapositivos e escolha **Duplicar Diapositivo**. A cópia tem os mesmos objetos.
2. Na cópia, mova um objeto, aumente-o, rode-o ou altere a cor. Também pode alterar o texto ou adicionar e eliminar objetos.
3. Clique na cópia e, em seguida, clique em **Transições > Transição para Este Diapositivo > Transformação**. Uma transformação demora 2 segundos. Pode alterar este valor com **Duração**.
4. Clique em **Pré-visualizar** para a ver ou inicie a apresentação de diapositivos.

### Que objetos ficam juntos

O JeffDeck emparelha um objeto do primeiro diapositivo com um objeto do segundo diapositivo. Tenta as seguintes formas, por esta ordem. Cada objeto fica, no máximo, num par.

1. **Um nome que começa por !!.** Dois objetos com o mesmo nome, cujo nome começa por dois pontos de exclamação, formam sempre um par. Podem ser quaisquer dois objetos, até um círculo e uma estrela.
2. **O mesmo nome.** Dois objetos do mesmo tipo com o mesmo nome formam um par. Uma caixa de texto e uma forma são tipos diferentes.
3. **O mesmo texto.** Duas caixas de texto com as mesmas palavras formam um par, sejam quais forem os nomes. O mesmo acontece com duas formas com as mesmas palavras.
4. **A mesma imagem.** Duas imagens da mesma fotografia ou ilustração formam um par, mesmo que tenham outro nome ou outro tamanho.
5. **O mesmo tipo de forma.** Dois círculos formam um par. O mesmo acontece com duas linhas ou duas tabelas.

Quando vários objetos podem formar um par, os que estão mais próximos uns dos outros são emparelhados primeiro.

Os nomes dos objetos vêm do ficheiro da apresentação. Uma apresentação criada noutro programa mantém os nomes que deu aos objetos e o JeffDeck utiliza-os.

### Como se move um par

- Dois objetos com a mesma forma são um único objeto que se move. A posição, o tamanho, a rotação, a cor de preenchimento, a cor da linha e a transparência mudam pouco a pouco. O recorte de uma imagem também muda.
- Dois objetos que não têm a mesma forma, como um círculo e um quadrado, deslocam-se para as mesmas posições enquanto um se transforma gradualmente no outro.
- Se o texto de um par não for igual, o texto antigo desvanece-se no texto novo.
- Os objetos que uma animação oculta são ignorados. Um objeto que uma animação moveu é considerado a partir do local onde terminou.

### O que se move: Objetos, Palavras ou Carateres

Clique em **Opções de Efeito** para escolher:

- **Objetos.** O texto de um par permanece no respetivo objeto. O texto antigo desvanece-se no novo.
- **Palavras.** Uma palavra que existe nos dois textos desloca-se da posição antiga para a nova. As outras palavras desaparecem e aparecem gradualmente. Um sinal como uma vírgula conta como uma palavra independente.
- **Carateres.** O mesmo, uma letra ou um algarismo de cada vez.

**Palavras** e **Carateres** funcionam em caixas de texto e formas. Um texto com mais de 16 000 carateres, texto escrito na vertical e texto numa forma invertida são reproduzidos como **Objetos**. O mesmo acontece com um par que não tenha nenhuma palavra ou carácter em comum.

A transformação também é uma transição noutros programas de apresentação (versões de 2019 e mais recentes). O JeffDeck lê-a e guarda-a da mesma forma que esses programas. Os programas mais antigos reproduzem uma transformação como um desvanecimento.

## Definir a temporização

O grupo **Temporização** define quanto tempo demora e como a apresentação sai do diapositivo.

- **Duração** é o número de segundos que o efeito demora. Uma nova transição demora 0,7 segundos. Pode introduzir valores de 0,01 a 59.
- **Ao Clicar com o Rato** está ativado por predefinição. A apresentação avança a partir deste diapositivo quando o apresentador clica. Desative esta opção para avançar com uma tecla, como Espaço ou Seta para a direita. As animações do diapositivo continuam a ser reproduzidas ao clicar.
- **Após** faz a apresentação avançar sozinha após o número de segundos que introduzir. Pode utilizar **Após** em conjunto com **Ao Clicar com o Rato**: a apresentação avança com o que acontecer primeiro.

**Aplicar a Todos** atribui a todos os diapositivos a transição e a temporização do diapositivo em que está.

Cada uma destas ações é um passo que Ctrl+Z anula.

## Transições de outros ficheiros

Uma apresentação criada noutro programa pode ter uma transição para a qual o JeffDeck não tem efeito. É reproduzida como um desvanecimento e guardada tal como estava, pelo que continua a funcionar no programa de origem.

## Relacionados

- [Separador Transições](tab-transitions)
- [Animações](animations)
- [Reproduzir a apresentação de diapositivos e utilizar a Vista de Apresentador](show)
