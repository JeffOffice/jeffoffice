# Ortografia, dicionário de sinónimos, pesquisa e hifenização
<!-- keywords: verificar ortografia, ortografia, corretor ortográfico, dicionário de sinónimos, sinónimos, pesquisar, pesquisa, dicionário, idioma de verificação, hifenização, hifenizar, traduzir, correção automática, palavra errada, sublinhado vermelho, idioma, gramática, verificar gramática, estilo, linha azul, linha pontilhada, verificar gramática à medida que escreve -->

O separador **Rever** tem ferramentas que o ajudam a escrever as palavras corretamente.

## Verificar ortografia à medida que escreve

O JeffPub coloca uma linha vermelha ondulada por baixo de uma palavra que não reconhece. Clique com o botão direito na palavra para escolher uma sugestão, escolha **Ignorar Tudo** (para esta sessão) ou escolha **Adicionar ao Dicionário**.

Para desativar esta opção, clique na seta ao lado de **Rever > Verificação > Ortografia** e clique em **Verificar Ortografia ao Escrever**. A mesma escolha está em **Ficheiro > Opções > Verificação**. É uma única opção para todas as aplicações JeffOffice e para todas as janelas, e o JeffPub recorda-a na próxima vez que o iniciar.

## As suas próprias palavras

Clique em **Ficheiro > Opções > Verificação > Dicionário Personalizado** para ver as palavras que acrescentou e para adicionar, remover, localizar, importar ou exportar palavras. A mesma caixa de diálogo tem uma lista de **Palavras Excluídas**: palavras que são sempre marcadas como erradas, mesmo que um dicionário as aceite, e que nunca são sugeridas. Uma palavra excluída é apenas essa forma exata, e não as suas outras formas; adicionar uma palavra ao dicionário retira-a da lista de excluídas. A importação e a exportação usam um ficheiro de texto simples, com uma palavra por linha (UTF-8). As suas listas são iguais em todas as aplicações JeffOffice e nunca são guardadas num documento.

## Verificar gramática à medida que escreve

O JeffPub também procura erros claros de gramática e de estilo no texto, em caixas de texto, formas e tabelas. Um problema recebe uma linha azul pontilhada por baixo, e uma palavra errada mantém a sua linha vermelha ondulada. Clique com o botão direito no problema para ver o que está errado e escolher uma sugestão, ou escolha **Ignorar Este Problema** ou **Ignorar Tudo**. O texto numa língua que não é verificada, e o texto marcado como "não verificar", é ignorado. Para desativar esta opção, clique em **Ficheiro > Opções > Verificação** e desmarque **Verificar gramática à medida que escreve**. É uma única opção para todas as aplicações JeffOffice.

**Que línguas são verificadas.** A gramática só é verificada em inglês, em todas as variantes regionais (americano, britânico, canadiano, australiano e os restantes). O texto em qualquer outra língua é verificado apenas quanto à ortografia e não recebe marcas de gramática. Um parágrafo é verificado na língua em que está marcado, e o texto sem língua é verificado como inglês, a menos que a maioria das suas palavras não sejam palavras inglesas, como numa passagem noutra língua que ninguém marcou. Os espaços a mais e os espaços antes de sinais de pontuação só são procurados em frases, e não em títulos, etiquetas, listas de nomes, células de tabelas que não sejam frases completas, ou citações. Num parágrafo com mais do que uma língua, só o inglês é verificado e o resto é ignorado. O texto que um ficheiro marca como "não verificar" também é ignorado.

**Escolher o que é procurado.** Clique em **Ficheiro > Opções > Verificação > Definições de gramática** para escolher os tipos de problemas: **Gramática** (verbos que não concordam, a forma errada de um verbo, "a" e "an"), **Pontuação** (espaços a mais ou em falta, vírgulas e hífenes), **Maiúsculas e minúsculas**, **Palavras confundidas** (palavras que soam de forma semelhante), **Repetição** e **Estilo**. O **Estilo** está desativado até o ativar. Este tipo sugere melhorias e não corrige erros: frases com mais de 40 palavras, a voz passiva, três frases seguidas que começam pela mesma palavra, e expressões prolixas ("in order to", "due to the fact that"). A escolha mantém-se quando clica em **OK** nas Opções. Aplica-se de imediato às linhas azuis e ao painel Ortografia (F7) nas janelas abertas desta aplicação, e às outras aplicações JeffOffice na próxima vez que forem iniciadas.

## Verificar o publicação inteira

1. Clique em **Rever > Verificação > Ortografia** ou prima F7.
2. A caixa de diálogo **Verificar Ortografia** mostra a primeira palavra com problema em **Não consta do dicionário**, com **Sugestões** por baixo.
3. Escolha uma sugestão ou escreva a sua em **Alterar para**.
4. Clique em **Alterar** para esta palavra ou em **Alterar Tudo** para todas as ocorrências. Clique em **Ignorar** para a saltar, em **Ignorar Tudo** para a saltar nesta sessão, ou em **Adicionar** para a guardar na sua lista de palavras.

O JeffPub verifica todas as caixas de texto, formas e tabelas, começando na primeira página. Quando **Verificar gramática à medida que escreve** está ativado, a verificação continua para os problemas de gramática, depois da ortografia, e mostra o que está errado em vez de **Não consta do dicionário**. Indica quando termina. Ignora palavras em MAIÚSCULAS e palavras com números. Pode alterar isso no separador **Verificação** de **Ficheiro > Opções** (janela **Opções do JeffPub**).

## Definir o idioma

A ortografia e a hifenização automática seguem o idioma do texto. O texto sem idioma definido é verificado como inglês (Estados Unidos). O JeffPub tem dicionários para inglês (Estados Unidos, Reino Unido, Canadá e Austrália), espanhol (21 variantes regionais, da Argentina à Venezuela, incluindo México e Espanha), francês, alemão, italiano, neerlandês e português (Brasil e Portugal).

1. Selecione o texto.
2. Clique em **Rever > Idioma > Definir Idioma de Verificação…**
3. Escolha um idioma. Escolha **Não verificar ortografia** para texto como nomes ou códigos.

## Encontrar uma palavra melhor

- **Dicionário de sinónimos:** clique numa palavra e clique em **Rever > Verificação > Dicionário de Sinónimos** (Shift+F7). Escolha um sinónimo, ou seja, uma palavra com um significado semelhante. Clique em **Inserir** para substituir a sua palavra. **Procurar** pesquisa o sinónimo que escolheu.
- **Pesquisar:** clique em **Rever > Verificação > Pesquisar** (Alt+F7) para abrir o painel **Pesquisar**. Escreva uma palavra em **Pesquisar por** e clique em **Procurar**.

Ambos usam um dicionário de sinónimos incorporado no JeffPub, apenas em inglês. Nada do que escreve é enviado para a Internet.

## Hifenização

A hifenização parte palavras longas com um hífen no fim de uma linha, para que as linhas fiquem uniformes. As novas caixas de texto são hifenizadas por predefinição. Para alterar isso para as novas caixas, veja **Hifenizar automaticamente em novas caixas de texto** no separador **Avançadas** de **Ficheiro > Opções** (janela **Opções do JeffPub**).

Para alterar um só artigo, clique na respetiva caixa de texto e clique em **Rever > Verificar > Hifenização…** (Ctrl+Shift+H). Também pode usar **Caixa de Texto > Texto > Hifenização**. Na caixa de diálogo **Hifenização**:

- **Hifenizar automaticamente este artigo** ativa ou desativa a hifenização.
- **Zona de hifenização** define quanto espaço vazio pode existir no fim de uma linha antes de o JeffPub partir uma palavra.
- **Inserir Hífen Opcional** acrescenta um hífen no cursor. Só é apresentado se a palavra for partida ali.
- **Adicionar Hífenes Opcionais** acrescenta um em cada sítio onde uma palavra longa pode ser partida.

## Traduzir texto

Selecione algum texto e clique em **Rever > Idioma > Traduzir**. O painel Traduzir traduz-no no seu computador e pode substituir o texto ou acrescentar a tradução por baixo dele. Veja [Traduzir texto](translate).

## Relacionados

- [Localizar e substituir texto](find-replace)
- [O separador Rever](tab-review)
- [Adicionar e formatar caixas de texto](text-boxes)
