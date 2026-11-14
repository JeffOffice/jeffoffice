# Traduzir texto
<!-- keywords: traduzir, tradução, tradutor, idioma, idiomas, espanhol, francês, alemão, italiano, português, neerlandês, russo, ucraniano, indonésio, offline, neste computador, transferir, modelos, substituir, inserir abaixo -->

**Rever > Idioma > Traduzir** abre o painel **Traduzir**. O painel traduz o seu texto no seu próprio computador. As suas palavras não são enviadas para lado nenhum, e não é necessária ligação à internet depois de os ficheiros de tradução de um idioma estarem no seu computador.

## Traduzir algum texto

1. Selecione o texto que pretende traduzir. Se não houver nada selecionado, o painel usa o parágrafo onde está o cursor. Numa caixa de texto selecionada, mas que não está a ser editada, o painel usa todo o texto da caixa.
2. Clique em **Rever > Idioma > Traduzir**. O painel mostra o seu texto em **Original**.
3. Em **De**, escolha o idioma em que está o seu texto. Em **Para**, escolha o idioma que pretende. O botão **Trocar** inverte os dois.
4. Clique em **Traduzir**. A tradução aparece em **Tradução**. As palavras a negrito, itálico e sublinhado mantêm o negrito, o itálico e o sublinhado.
5. Clique em **Substituir** para colocar a tradução no lugar do seu texto, ou em **Inserir Abaixo** para a adicionar em novos parágrafos abaixo do seu texto. Cada uma destas ações é um único passo, por isso **Anular** a desfaz.

Se mover o cursor ou alterar o texto depois de clicar em **Traduzir**, clique em **Utilizar Seleção** para ler o texto novamente e, em seguida, clique em **Traduzir** outra vez. O painel não substitui uma tradução por texto que já não é o texto que foi traduzido.

**Cancelar** interrompe uma tradução que esteja a demorar demasiado, e fechar o painel também a interrompe: a janela nunca fica à espera. Textos longos demoram mais: uma página de texto demora alguns segundos na maioria dos computadores. O painel traduz, de cada vez, no máximo 1 000 000 de letras (cerca de 200 páginas); se tiver mais texto selecionado, indica-o e pede que selecione menos. Os parágrafos longos são divididos em partes de cerca de 1 500 letras, no fim de uma frase sempre que possível, e depois são novamente juntos.

## A primeira vez que usa um idioma

Uma tradução precisa de um conjunto de ficheiros, chamado modelo, para cada direção: um para do inglês para o alemão, outro para do alemão para o inglês. Cada modelo tem cerca de 37 MB. Os ficheiros não fazem parte do JeffPub. Na primeira vez que traduzir com um idioma, o painel indica o que é necessário e o respetivo tamanho, por exemplo "Transferir alemão (74 MB)?". Nada é transferido até clicar nesse botão. O painel mostra o progresso da transferência, e **Cancelar** interrompe-a e não deixa nada para trás.

O JeffPub verifica cada ficheiro através de uma impressão digital (uma soma de verificação SHA-256) antes de o utilizar. Um ficheiro que não corresponda é descartado, e o idioma não é instalado.

Os ficheiros são guardados numa única pasta partilhada por todas as aplicações do JeffOffice (uma pasta chamada JeffOffice na pasta de dados do seu utilizador, em Translation/models), por isso um idioma transferido numa aplicação fica disponível nas outras, e é transferido apenas uma vez. Dois idiomas que não têm modelo próprio, como alemão e francês, passam pelo inglês, pelo que o painel precisa dos dois modelos.

## Quando um idioma não tem modelo

Se um par de idiomas não tiver modelo, o painel indica-o e oferece o botão **Abrir no Seu Navegador em Alternativa**. Esse botão abre um site de tradução gratuito, o LibreTranslate (ou o serviço cujo endereço escolheu nas definições para esse fim), com o seu texto. Só então o seu texto sai do computador, porque é enviado para esse site, e o painel deixa de indicar que o texto fica no seu computador. Nada é enviado até clicar nesse botão.

## Sobre a qualidade

Os modelos são pequenos, por isso funcionam depressa num computador normal. Traduzem bem frases e pior etiquetas curtas: uma única palavra ou o nome de um botão pode sair errado. Leia a tradução antes de a utilizar. Nomes, números e tabulações são mantidos como estavam.

Imagens, tabulações e quebras de linha dentro de um parágrafo ficam como estão. A tradução preenche as palavras entre elas.

## Relacionados

- [Ortografia, dicionário de sinónimos, pesquisa e hifenização](proofing)
